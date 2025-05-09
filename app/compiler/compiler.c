#include "bytecode/chunk.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

#include <bytecode/object.h>
#include <scanner/scanner.h>
#include <scanner/token.h>
#include <string.h>
#include <utils/array.h>

#ifdef DEBUG_TRACE_EXECUTION
#include <bytecode/debug/debug.h>
#endif

#define UINT8_COUNT       (UINT8_MAX + 1)
#define DUMMY_LOCAL_INDEX 0
#define UNINTIALIZED      -1

typedef struct
{
    token_t current;
    token_t previous;
    bool    had_error;
    bool    panic_mode;
} parser_t;

typedef struct
{
    int     depth;
    token_t name;
} local_t;

typedef struct
{
    u8      local_count;
    u8      scope_depth;
    local_t locals[UINT8_COUNT];
} scope_t;

typedef struct
{
    parser_t       parser;
    scanner_t      scanner;
    chunk_t* const chunk;
    object_t*      objects;
    table_t*       strings;
    scope_t        current_scope;
} compiler_t;

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} precedence_t;

// See parse_precedence for relevance of can_assign
typedef void (*parse_fn_t)(compiler_t* const, const bool can_assign);

typedef struct
{
    parse_fn_t   prefix;
    parse_fn_t   infix;
    precedence_t precedence;

} parser_rule_t;

static void error_at(const token_t token, str message)
{
    fprintf(stderr, "[Line %zu] Error", token.line);
    if (token.type == TOKEN_EOF) { fprintf(stderr, " at end"); }
    else if (token.type == TOKEN_ERROR)
    {
        // Nothing.
    }
    else
    {
        fprintf(stderr, " at '");
        for (usize i = 0, len = token.length; i < len; ++i)
        {
            fprintf(stderr, "%c", token.start[i]);
        }
        fprintf(stderr, "'");
    }

    fprintf(stderr, ": %s\n", message);
}

static void error(parser_t* const parser, str message, bool previous)
{
    if (!parser->panic_mode)
    {
        parser->panic_mode = true;
        error_at(previous ? parser->previous : parser->current, message);
    }
    parser->had_error = true;
}

static void advance_compiler(compiler_t* const self)
{
    self->parser.previous = self->parser.current;

    while (true)
    {
        self->parser.current = scan_token(&self->scanner);
        if (self->parser.current.type != TOKEN_ERROR) { break; }
        error(&self->parser, self->parser.current.start, false);
    }
}

static bool check(parser_t* const parser, const token_type_t type)
{
    return parser->current.type == type;
}

static void consume(compiler_t* const self, const token_type_t type, str message)
{
    if (check(&self->parser, type))
    {
        advance_compiler(self);
        return;
    }

    error(&self->parser, message, false);
}

static bool match(compiler_t* const self, const token_type_t type)
{
    if (!check(&self->parser, type)) { return false; }

    advance_compiler(self);

    return true;
}

static void emit_byte(compiler_t* const self, const u8 byte, const usize line)
{
    write_chunk(self->chunk, byte, (u32)line);
}

static void emit_bytes(compiler_t* const self, const u8 first_byte, const u8 second_byte)
{
    const u32 line = (u32)self->parser.previous.line;
    write_chunk(self->chunk, first_byte, line);
    write_chunk(self->chunk, second_byte, line);
}

static u8 make_constant(compiler_t* const self, const value_t value)
{
    const usize constant = add_constant(self->chunk, value);

    if (constant > BYTE_MAX)
    {
        error(&self->parser, "Too many constants in single chunk", true);
        return (u8)constant;
    }

    return (u8)constant;
}

static void emit_constant(compiler_t* const self, const value_t value)
{
    const u8 index = make_constant(self, value);

    emit_bytes(self, OP_CONSTANT, index);
}

// NOLINTNEXTLINE(misc-unused-parameters)
static void number(compiler_t* const self, const bool _)
{
    const f64 value = strtod(self->parser.previous.start, NULL);
    emit_constant(self, from_number(value));
}

// NOLINTNEXTLINE(misc-unused-parameters)
static void string(compiler_t* const self, const bool _)
{
    // The +1 is to skip the first quote.
    // The -2 is to remove the quotes from the string.
    //  ""hello"" -> hello
    emit_constant(self,
                  from_object((object_t*)copy_string(self->parser.previous.start + 1UL,
                                                     self->parser.previous.length - 2UL,
                                                     self->objects,
                                                     self->strings)));
}

static u8 identifier_constant(compiler_t* const self, const token_t* const name)
{
    return make_constant(self,
                         from_object((object_t*)copy_string(
                             name->start, name->length, self->objects, self->strings)));
}

static void add_local(compiler_t* const self, const token_t name)
{
    if (self->current_scope.local_count == (UINT8_COUNT - 1))
    {
        error(&self->parser, "Too many variables in scope.", true);
    }

    local_t* const local = &self->current_scope.locals[self->current_scope.local_count++];

    local->name          = name;

    local->depth         = UNINTIALIZED;
}

static bool identifiers_equal(const token_t* const a, const token_t* const b)
{
    return a->length == b->length && memcmp(a->start, b->start, a->length) == 0;
}

static int resolve_local(compiler_t* const self, const token_t* const name)
{
    scope_t* const scope = &self->current_scope;

    for (u8 i = scope->local_count - 1; i >= 0; --i)
    {
        local_t* const local = &scope->locals[i];

        if (identifiers_equal(name, &local->name))
        {
            if (local->depth == UNINTIALIZED)
            {
                error(&self->parser,
                      "Can't read local variable in its own initializer.",
                      true);
            }
            return i;
        }
    }

    return -1;
}

static void declare_variable(compiler_t* const self)
{
    if (self->current_scope.scope_depth == 0) { return; }

    const token_t* const name = &self->parser.previous;

    for (usize i = self->current_scope.local_count - 1; i >= 0; --i)
    {
        const local_t* const local = &self->current_scope.locals[i];

        if (local->depth != UNINTIALIZED &&
            local->depth < self->current_scope.scope_depth)
        {
            break;
        }

        if (identifiers_equal(name, &local->name))
        {
            error(&self->parser,
                  "A variable with the same name in this scope already exists.",
                  true);
        }
    }

    add_local(self, *name);
}

static void expression(compiler_t* self);

static void named_variable(compiler_t* const self, const bool can_assign)
{
    u8  get_op = 0;
    u8  set_op = 0;

    int arg    = resolve_local(self, &self->parser.previous);

    if (arg != UNINTIALIZED)
    {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    }
    else
    {
        arg    = (int)identifier_constant(self, &self->parser.previous);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (can_assign && match(self, TOKEN_EQUAL))
    {
        expression(self);
        emit_bytes(self, set_op, (u8)arg);
    }
    else { emit_bytes(self, get_op, (u8)arg); }
}

static void variable(compiler_t* const self, const bool can_assign)
{
    named_variable(self, can_assign);
}

static parser_rule_t get_rule(token_type_t type);

static void parse_precedence(compiler_t* const self, const precedence_t precedence)
{
    advance_compiler(self);

    const parse_fn_t prefix_rule = get_rule(self->parser.previous.type).prefix;

    if (!prefix_rule)
    {
        error(&self->parser, "Expected expression.", true);
        return;
    }

    const bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(self, can_assign);

    while (precedence <= get_rule(self->parser.current.type).precedence)
    {
        advance_compiler(self);

        const parse_fn_t infix_rule = get_rule(self->parser.previous.type).infix;

        infix_rule(self, can_assign);

        if (can_assign && match(self, TOKEN_EQUAL))
        {
            /* Assumes assignment is not in a valid context.
               A valid context is then a top level expression statement
               Or as part of an assignment expression: `var a = 1; a = 2;`
               This is to prevent things such as `a * b = c + d;`
            */
            error(&self->parser, "Invalid assignment target.", true);
        }
    }
}

static u8 parse_variable(compiler_t* self, str error_message)
{
    consume(self, TOKEN_IDENTIFIER, error_message);

    declare_variable(self);

    if (self->current_scope.scope_depth > 0) { return DUMMY_LOCAL_INDEX; }

    return identifier_constant(self, &self->parser.previous);
}

static void mark_initialized(scope_t* const scope)
{
    scope->locals[scope->local_count - 1].depth = scope->scope_depth;
}

static void define_variable(compiler_t* const self, const u8 global)
{
    if (self->current_scope.scope_depth > 0)
    {
        mark_initialized(&self->current_scope);
        return;
    }

    emit_bytes(self, OP_DEFINE_GLOBAL, global);
}

// NOLINTNEXTLINE(misc-unused-parameters)
static void unary(compiler_t* const self, const bool _)
{
    const token_type_t operator_type = self->parser.previous.type;
    const usize        line          = self->parser.previous.line;

    parse_precedence(self, PREC_UNARY);

    switch (operator_type)
    {
        case TOKEN_MINUS: emit_byte(self, OP_NEGATE, line); break;
        case TOKEN_BANG: emit_byte(self, OP_NOT, line);
        default: return;
    }
}

static void expression(compiler_t* const self)
{
    parse_precedence(self, PREC_ASSIGNMENT);
}

static void var_declaration(compiler_t* const self)
{
    const u8 global = parse_variable(self, "Expected variable name.");

    match(self, TOKEN_EQUAL) ? expression(self)
                             : emit_byte(self, OP_NIL, self->parser.previous.line);

    consume(self, TOKEN_SEMICOLON, "Expected ';' after variable declaration.");

    define_variable(self, global);
}

static void statement(compiler_t* self);

static void expression_statement(compiler_t* self);

static void print_statement(compiler_t* const self)
{
    const usize line = self->parser.previous.line;
    expression(self);
    consume(self, TOKEN_SEMICOLON, "Expected ';' after value.");
    emit_byte(self, OP_PRINT, line);
}

static void synchronize(compiler_t* const self)
{
    self->parser.panic_mode = false;

    while (self->parser.current.type != TOKEN_EOF)
    {
        if (self->parser.previous.type == TOKEN_SEMICOLON) { return; }

        switch (self->parser.current.type)
        {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN: return;

            default:;
        }

        advance_compiler(self);
    }
}

static void declaration(compiler_t* const self)
{
    match(self, TOKEN_VAR) ? var_declaration(self) : statement(self);

    if (self->parser.panic_mode) { synchronize(self); }
}

static void begin_scope(scope_t* const scope) { ++scope->scope_depth; }

static void block(compiler_t* const self)
{
    while (!check(&self->parser, TOKEN_RIGHT_BRACE) && !check(&self->parser, TOKEN_EOF))
    {
        declaration(self);
    }

    consume(self, TOKEN_RIGHT_BRACE, "Expected '}' after block.");
}

static void end_scope(compiler_t* const self)
{
    scope_t* const scope = &self->current_scope;

    --scope->scope_depth;

    u8 destroyed_local_count = 0;

    while (scope->local_count > 0 &&
           scope->locals[scope->local_count - 1].depth > scope->scope_depth)
    {
        ++destroyed_local_count;
        --scope->local_count;
    }

    if (destroyed_local_count > 1) { emit_bytes(self, OP_POPN, destroyed_local_count); }
    else { emit_byte(self, OP_POP, self->parser.previous.line); }
}

void statement(compiler_t* const self)
{
    if (match(self, TOKEN_PRINT)) { print_statement(self); }
    else if (match(self, TOKEN_LEFT_BRACE))
    {
        begin_scope(&self->current_scope);
        block(self);
        end_scope(self);
    }
    else { expression_statement(self); }
}

static void expression_statement(compiler_t* const self)
{
    expression(self);
    const usize line = self->parser.previous.line;
    consume(self, TOKEN_SEMICOLON, "Expected ';' after expression.");
    emit_byte(self, OP_POP, line);
}

// NOLINTNEXTLINE(misc-unused-parameters)
static void grouping(compiler_t* const self, const bool _)
{
    expression(self);
    consume(self, TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

static void          binary(compiler_t* self, bool _);

static void          literal(compiler_t* self, bool _);

static parser_rule_t get_rule(const token_type_t type)
{
    const static parser_rule_t rules[] = {
        [TOKEN_LEFT_PAREN]    = {grouping, NULL, PREC_NONE},
        [TOKEN_RIGHT_PAREN]   = {NULL, NULL, PREC_NONE},
        [TOKEN_LEFT_BRACE]    = {NULL, NULL, PREC_NONE},
        [TOKEN_RIGHT_BRACE]   = {NULL, NULL, PREC_NONE},
        [TOKEN_COMMA]         = {NULL, NULL, PREC_NONE},
        [TOKEN_DOT]           = {NULL, NULL, PREC_NONE},
        [TOKEN_MINUS]         = {unary, binary, PREC_TERM},
        [TOKEN_PLUS]          = {NULL, binary, PREC_TERM},
        [TOKEN_SEMICOLON]     = {NULL, NULL, PREC_NONE},
        [TOKEN_SLASH]         = {NULL, binary, PREC_FACTOR},
        [TOKEN_STAR]          = {NULL, binary, PREC_FACTOR},
        [TOKEN_BANG]          = {unary, NULL, PREC_NONE},
        [TOKEN_BANG_EQUAL]    = {NULL, binary, PREC_EQUALITY},
        [TOKEN_EQUAL]         = {NULL, NULL, PREC_NONE},
        [TOKEN_EQUAL_EQUAL]   = {NULL, binary, PREC_EQUALITY},
        [TOKEN_GREATER]       = {NULL, binary, PREC_COMPARISON},
        [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
        [TOKEN_LESS]          = {NULL, binary, PREC_COMPARISON},
        [TOKEN_LESS_EQUAL]    = {NULL, binary, PREC_COMPARISON},
        [TOKEN_IDENTIFIER]    = {variable, NULL, PREC_NONE},
        [TOKEN_STRING]        = {string, NULL, PREC_NONE},
        [TOKEN_NUMBER]        = {number, NULL, PREC_NONE},
        [TOKEN_AND]           = {NULL, NULL, PREC_NONE},
        [TOKEN_CLASS]         = {NULL, NULL, PREC_NONE},
        [TOKEN_ELSE]          = {NULL, NULL, PREC_NONE},
        [TOKEN_FALSE]         = {literal, NULL, PREC_NONE},
        [TOKEN_FOR]           = {NULL, NULL, PREC_NONE},
        [TOKEN_FUN]           = {NULL, NULL, PREC_NONE},
        [TOKEN_IF]            = {NULL, NULL, PREC_NONE},
        [TOKEN_NIL]           = {literal, NULL, PREC_NONE},
        [TOKEN_OR]            = {NULL, NULL, PREC_NONE},
        [TOKEN_PRINT]         = {NULL, NULL, PREC_NONE},
        [TOKEN_RETURN]        = {NULL, NULL, PREC_NONE},
        [TOKEN_SUPER]         = {NULL, NULL, PREC_NONE},
        [TOKEN_THIS]          = {NULL, NULL, PREC_NONE},
        [TOKEN_TRUE]          = {literal, NULL, PREC_NONE},
        [TOKEN_VAR]           = {NULL, NULL, PREC_NONE},
        [TOKEN_WHILE]         = {NULL, NULL, PREC_NONE},
        [TOKEN_ERROR]         = {NULL, NULL, PREC_NONE},
        [TOKEN_EOF]           = {NULL, NULL, PREC_NONE},
    };

    return rules[type];
}

// NOLINTNEXTLINE(misc-unused-parameters)
void binary(compiler_t* const self, const bool _)
{
    const token_type_t  operator_type = self->parser.previous.type;
    const usize         line          = self->parser.previous.line;

    const parser_rule_t rule          = get_rule(operator_type);

    parse_precedence(self, (precedence_t)(rule.precedence + 1));

    switch (operator_type)
    {
        case TOKEN_BANG_EQUAL: emit_bytes(self, OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL: emit_byte(self, OP_EQUAL, line); break;
        case TOKEN_GREATER: emit_byte(self, OP_GREATER, line); break;
        case TOKEN_GREATER_EQUAL: emit_bytes(self, OP_LESS, OP_NOT); break;
        case TOKEN_LESS: emit_byte(self, OP_LESS, line); break;
        case TOKEN_LESS_EQUAL: emit_bytes(self, OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS: emit_byte(self, OP_ADD, line); break;
        case TOKEN_MINUS: emit_byte(self, OP_SUBTRACT, line); break;
        case TOKEN_STAR: emit_byte(self, OP_MULTIPLY, line); break;
        case TOKEN_SLASH: emit_byte(self, OP_DIVIDE, line); break;
        default: return;
    }
}

// NOLINTNEXTLINE(misc-unused-parameters)
void literal(compiler_t* const self, const bool _)
{
    const usize token_line = self->parser.previous.line;

    switch (self->parser.previous.type)
    {
        case TOKEN_TRUE: emit_byte(self, OP_TRUE, token_line); break;

        case TOKEN_FALSE: emit_byte(self, OP_FALSE, token_line); break;

        case TOKEN_NIL: emit_byte(self, OP_NIL, token_line); break;

        default: return;
    }
}

bool compile(str const       source,
             chunk_t*        chunk,
             object_t* const objects,
             table_t* const  strings)
{
    compiler_t compiler = {.scanner = init_scanner(source),
                           .chunk   = chunk,
                           .objects = objects,
                           .strings = strings};

    advance_compiler(&compiler);

    while (!match(&compiler, TOKEN_EOF)) { declaration(&compiler); }

    emit_byte(&compiler, OP_RETURN, compiler.parser.previous.line);

    const bool parser_had_no_error = !compiler.parser.had_error;

#if DEBUG_TRACE_EXECUTION
    if (parser_had_no_error) { disassemble_chunk(compiler.chunk, "code"); }
#endif

    return parser_had_no_error;
}

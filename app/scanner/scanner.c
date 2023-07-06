#include <string.h>

#include "scanner.h"

bool is_digit(const char c) { return c >= '0' && c <= '9'; }

bool is_alpha(const char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_at_end(const scanner_t* const self) { return *self->current == '\0'; }

token_t make_token(const scanner_t* const self, const token_type_t type)
{
    const token_t token = {.type   = type,
                           .start  = self->start,
                           .length = (size_t)(self->current - self->start),
                           .line   = self->line};

    return token;
}

token_t error_token(const scanner_t* const self, str message)
{
    const token_t token = {.type   = TOKEN_ERROR,
                           .start  = message,
                           .length = (size_t)strlen(message),
                           .line   = self->line};

    return token;
}

char advance(scanner_t* const self)
{
    ++self->current;
    return self->current[-1];
}

void advance_(scanner_t* const self) { ++self->current; }

bool match(scanner_t* const self, const char expected)
{
    if (is_at_end(self) || *self->current != expected) { return false; }

    advance_(self);

    return true;
}

char peek(const scanner_t* const self) { return *self->current; }

char peek_next(const scanner_t* const self)
{
    return is_at_end(self) ? '\0' : self->current[1UL];
}

void skip_whitespace(scanner_t* const self)
{
    while (true)
    {
        switch (peek(self))
        {
            case ' ':
            case '\r':
            case '\t': advance_(self); break;
            case '\n':
            {
                ++self->line;
                advance_(self);
                break;
            }
            case '/':
            {
                if (peek_next(self) == '/')
                {
                    while (peek(self) != '\n' && !is_at_end(self))
                    {
                        advance_(self);
                    }
                }
                else { return; }
            }
            default: return;
        }
    }
}

token_t string(scanner_t* const self)
{
    while (peek(self) != '"' && !is_at_end(self))
    {
        if (peek(self) == '\n') { ++self->line; }
        advance_(self);
    }

    if (is_at_end(self)) { return error_token(self, "Unterminated string."); }

    // The closing quotes
    advance_(self);

    return make_token(self, TOKEN_STRING);
}

token_t number(scanner_t* const self)
{
    while (is_digit(peek(self))) { advance_(self); }

    if (peek(self) == '.' && is_digit(peek_next(self)))
    {
        advance_(self);

        while (is_digit(peek(self))) { advance_(self); }
    }

    return make_token(self, TOKEN_NUMBER);
}

token_type_t check_keyword(scanner_t* const   self,
                           const size_t       start,
                           const size_t       length,
                           str                rest,
                           const token_type_t type)
{
    const bool matches = self->current - self->start == start + length &&
                         memcmp(self->start + start, rest, length) == 0;

    return matches ? type : TOKEN_IDENTIFIER;
}

token_type_t identifier_type(scanner_t* const self)
{
    switch (self->start[0])
    {
        case 'a': return check_keyword(self, 1, 2, "nd", TOKEN_AND);
        case 'c': return check_keyword(self, 1, 4, "lass", TOKEN_CLASS);
        case 'e': return check_keyword(self, 1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (self->current - self->start > 1)
            {
                switch (self->start[1])
                {
                    case 'a':
                        return check_keyword(self, 2, 3, "lse", TOKEN_FALSE);
                    case 'o': return check_keyword(self, 2, 3, "r", TOKEN_FOR);
                    case 'u': return check_keyword(self, 2, 3, "n", TOKEN_FUN);
                }
            }
            break;
        case 'i': return check_keyword(self, 1, 1, "f", TOKEN_IF);
        case 'n': return check_keyword(self, 1, 2, "il", TOKEN_NIL);
        case 'o': return check_keyword(self, 1, 1, "r", TOKEN_OR);
        case 'p': return check_keyword(self, 1, 4, "rint", TOKEN_PRINT);
        case 'r': return check_keyword(self, 1, 5, "eturn", TOKEN_RETURN);
        case 's': return check_keyword(self, 1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (self->current - self->start > 1)
            {
                switch (self->start[1])
                {
                    case 'h':
                        return check_keyword(self, 2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return check_keyword(self, 2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v': return check_keyword(self, 1, 2, "ar", TOKEN_VAR);
        case 'w': return check_keyword(self, 1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

token_t identifier(scanner_t* const self)
{
    while (is_alpha(peek(self)) || is_digit(peek(self))) { advance_(self); }

    return make_token(self, identifier_type(self));
}

scanner_t init_scanner(str source)
{
    const scanner_t self = {.start = source, .current = source, .line = 1UL};

    return self;
}

token_t scan_token(scanner_t* const self)
{
    skip_whitespace(self);

    self->start = self->current;

    if (is_at_end(self)) { return make_token(self, TOKEN_EOF); }

    const char c = advance(self);

    if (is_digit(c)) { return number(self); }

    if (is_alpha(c)) { return identifier(self); }

    switch (c)
    {
        case '(': return make_token(self, TOKEN_LEFT_PAREN);
        case ')': return make_token(self, TOKEN_RIGHT_PAREN);
        case '{': return make_token(self, TOKEN_LEFT_BRACE);
        case '}': return make_token(self, TOKEN_RIGHT_BRACE);
        case ';': return make_token(self, TOKEN_SEMICOLON);
        case ',': return make_token(self, TOKEN_COMMA);
        case '.': return make_token(self, TOKEN_DOT);
        case '-': return make_token(self, TOKEN_MINUS);
        case '+': return make_token(self, TOKEN_PLUS);
        case '/': return make_token(self, TOKEN_SLASH);
        case '*': return make_token(self, TOKEN_STAR);
        case '!':
            return make_token(self,
                              match(self, '=' ? TOKEN_BANG_EQUAL : TOKEN_BANG));
        case '=':
            return make_token(
                self, match(self, '=' ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL));
        case '<':
            return make_token(self,
                              match(self, '=' ? TOKEN_LESS_EQUAL : TOKEN_LESS));
        case '>':
            return make_token(
                self, match(self, '=' ? TOKEN_GREATER_EQUAL : TOKEN_GREATER));
        case '"': return string(self);
    }

    return error_token(self, "Unexpected character.");
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lex.h"

lex *lex_init_string(char *input)
{
    lex *l = malloc(sizeof(lex));
    l->input = malloc(sizeof(char) * strlen(input) + 10);
    strcpy(l->input, input);
    
    l->index = 0;
    l->buf_index = 0;
    l->token_stack_len = 0;
    
    for (int i = 0; i<1024; i++)
        l->buffer[i] = 0;
    
    return l;
}

void lex_set_last_buffer(lex *l)
{
    l->last_buf_len = l->buf_index;
    for (int i = 0; i<l->buf_index + 1; i++) {
        l->last_buf[i] = l->buffer[i];
    }
}

void lex_clear_buffer(lex *l)
{
    for (int i = 0; i<l->buf_index + 1; i++) {
        l->buffer[i] = 0;
    }
    
    l->buf_index = 0;
}

int lex_is_symbol(char c)
{
    switch (c) {
        case '.':
        case ';':
        case ',':
        case '(': case ')':
        case '[': case ']':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '&': case '|': case '^':
        case ':':
        case '>': case '<':
        case '=': case '!': return 1;
        
        default: {}
    }
    return 0;
}

token lex_get_symbol(lex *l, char c)
{
    switch (c) {
        case '.': return t_dot;
        case ';': return t_semicolon;
        case ',': return t_comma;
        case '(': return t_lparen;
        case ')': return t_rparen;
        case '[': return t_lbracket;
        case ']': return t_rbracket;
        case '+': return t_plus;
        case '*': return t_mul;
        case '/': return t_div;
        case '%': return t_mod;
        case '&': return t_and;
        case '|': return t_or;
        case '^': return t_xor;
        case '=': return t_eq;
        
        case '-': {
            char c2 = l->input[l->index];
            if (c2 == '>') {
                ++l->index;
                return t_arrow;
            }
            return t_minus;
        } break;
        
        case ':': {
            char c2 = l->input[l->index];
            if (c2 == '=') {
                ++l->index;
                return t_assign;
            }
            return t_colon;
        } break;
        
        case '>': {
            char c2 = l->input[l->index];
            if (c2 == '=') {
                ++l->index;
                return t_gte;
            }
            return t_gt;
        } break;
        
        case '<': {
            char c2 = l->input[l->index];
            if (c2 == '=') {
                ++l->index;
                return t_lte;
            }
            return t_lt;
        } break;
        
        case '!': {
            char c2 = l->input[l->index];
            if (c2 == '=') {
                ++l->index;
                return t_neq;
            }
            return t_none;
        } break;
        
        default: {}
    }
    
    return t_none;
}

int lex_is_int(lex *l)
{
    if (l->buffer[0] == '0' && l->buffer[1] == 'x') {
        return 1;
    }

    for (int i = 0; i<l->buf_index; i++) {
        if (isdigit(l->buffer[i]) == 0) return 0;
    }

    return 1;
}

uint64_t lex_get_int(lex *l)
{
    if (l->last_buf[0] == '0' && l->last_buf[1] == 'x') {
        return (uint64_t)strtol(l->last_buf, NULL, 0);
    }

    return atoi(l->last_buf);
}

char *lex_get_id(lex *l)
{
    char *str = malloc(sizeof(char) * (l->last_buf_len + 1));
    int i;
    
    for (i = 0; i<l->last_buf_len; i++) {
        str[i] = l->last_buf[i];
    }
    str[i] = 0;
    
    return str;
}

void lex_rewind(lex *l, token tk)
{
    ++l->token_stack_len;
    l->token_stack[l->token_stack_len] = tk;
}

token lex_get_next(lex *l)
{
    if (l->token_stack_len > 0) {
        token sym = l->token_stack[l->token_stack_len];
        --l->token_stack_len;
        if (sym != t_none)
            return sym;
    }

    if (l->index >= strlen(l->input)) {
        return t_eof;
    }
    
    while (l->index < strlen(l->input)) {
        char c = l->input[l->index];
        ++l->index;
        if (c == '#') {
            while (c != '\n') {
                c = l->input[l->index];
                ++l->index;
            }
        }
        
        if (c == '\"') {
            do {
                c = l->input[l->index];
                ++l->index;
                
                if (c != '\"') {
                    l->buffer[l->buf_index] = c;
                    ++l->buf_index;
                }
            } while (c != '\"');
            
            lex_set_last_buffer(l);
            lex_clear_buffer(l);
            return t_string_literal;
        }
        
        if (c == '\'') {
            char c = l->input[l->index];
            if (c == '\\') {
                char c2 = c;
                if (l->input[l->index+1] == 'n') {
                    ++l->index;
                    l->buffer[0] = '\n';
                } else {
                    l->buffer[0] = c;
                }
            } else {
                l->buffer[0] = c;
            }
            ++l->buf_index;
            ++l->index;
            
            ++l->index;
            
            lex_set_last_buffer(l);
            lex_clear_buffer(l);
            return t_char_literal;
        }
        
        if (c == ' ' || c == '\n' || lex_is_symbol(c)) {
            if (lex_is_symbol(c)) {
                token sym = lex_get_symbol(l, c);
                if (sym == t_none) continue;
                
                if (strlen(l->buffer) > 0) {
                    ++l->token_stack_len;
                    l->token_stack[l->token_stack_len] = sym;
                } else {
                    return sym;
                }
            }
            
            if (strlen(l->buffer) == 0) {
                continue;
            }
            
            if (strcmp(l->buffer, "extern") == 0) {
                lex_clear_buffer(l);
                return t_extern;
            } else if (strcmp(l->buffer, "func") == 0) {
                lex_clear_buffer(l);
                return t_func;
            } else if (strcmp(l->buffer, "struct") == 0) {
                lex_clear_buffer(l);
                return t_struct;
            } else if (strcmp(l->buffer, "end") == 0) {
                lex_clear_buffer(l);
                return t_end;
            } else if (strcmp(l->buffer, "return") == 0) {
                lex_clear_buffer(l);
                return t_return;
            
            } else if (strcmp(l->buffer, "var") == 0) {
                lex_clear_buffer(l);
                return t_var;
            } else if (strcmp(l->buffer, "const") == 0) {
                lex_clear_buffer(l);
                return t_const;
            
            } else if (strcmp(l->buffer, "bool") == 0) {
                lex_clear_buffer(l);
                return t_bool;
            } else if (strcmp(l->buffer, "char") == 0) {
                lex_clear_buffer(l);
                return t_char;
            } else if (strcmp(l->buffer, "string") == 0) {
                lex_clear_buffer(l);
                return t_string;
            } else if (strcmp(l->buffer, "i8") == 0) {
                lex_clear_buffer(l);
                return t_i8;
            } else if (strcmp(l->buffer, "u8") == 0) {
                lex_clear_buffer(l);
                return t_u8;
            } else if (strcmp(l->buffer, "i16") == 0) {
                lex_clear_buffer(l);
                return t_i16;
            } else if (strcmp(l->buffer, "u16") == 0) {
                lex_clear_buffer(l);
                return t_u16;
            } else if (strcmp(l->buffer, "i32") == 0) {
                lex_clear_buffer(l);
                return t_i32;
            } else if (strcmp(l->buffer, "u32") == 0) {
                lex_clear_buffer(l);
                return t_u32;
            } else if (strcmp(l->buffer, "i64") == 0) {
                lex_clear_buffer(l);
                return t_i64;
            } else if (strcmp(l->buffer, "u64") == 0) {
                lex_clear_buffer(l);
                return t_u64;
            
            } else if (strcmp(l->buffer, "if") == 0) {
                lex_clear_buffer(l);
                return t_if;
            } else if (strcmp(l->buffer, "elif") == 0) {
                lex_clear_buffer(l);
                return t_elif;
            } else if (strcmp(l->buffer, "else") == 0) {
                lex_clear_buffer(l);
                return t_else;
            } else if (strcmp(l->buffer, "while") == 0) {
                lex_clear_buffer(l);
                return t_while;
                
            } else if (strcmp(l->buffer, "is") == 0) {
                lex_clear_buffer(l);
                return t_is;
            } else if (strcmp(l->buffer, "then") == 0) {
                lex_clear_buffer(l);
                return t_then;
            } else if (strcmp(l->buffer, "do") == 0) {
                lex_clear_buffer(l);
                return t_do;
            
            } else if (strcmp(l->buffer, "break") == 0) {
                lex_clear_buffer(l);
                return t_break;
            } else if (strcmp(l->buffer, "continue") == 0) {
                lex_clear_buffer(l);
                return t_continue;
            
            } else if (strcmp(l->buffer, "import") == 0) {
                lex_clear_buffer(l);
                return t_import;
                
            } else if (strcmp(l->buffer, "true") == 0) {
                lex_clear_buffer(l);
                return t_true;
            } else if (strcmp(l->buffer, "false") == 0) {
                lex_clear_buffer(l);
                return t_false;
            
            } else if (strcmp(l->buffer, "and") == 0) {
                lex_clear_buffer(l);
                return t_lgand;
            } else if (strcmp(l->buffer, "or") == 0) {
                lex_clear_buffer(l);
                return t_lgor;
                
            } else if (lex_is_int(l)) {
                lex_set_last_buffer(l);
                lex_clear_buffer(l);
                return t_int_literal;
            } else {
                lex_set_last_buffer(l);
                lex_clear_buffer(l);
                return t_id;
            }
        } else {
            l->buffer[l->buf_index] = c;
            ++l->buf_index;
        }
    }
    
    return t_eof;
}

void lex_debug(token t, lex *l)
{
    switch (t) {
        case t_eof: puts("EOF"); break;
        
        case t_extern: puts("EXTERN"); break;
        case t_func: puts("FUNC"); break;
        case t_struct: puts("STRUCT"); break;
        case t_end: puts("END"); break;
        case t_return: puts("RETURN"); break;
        case t_var: puts("VAR"); break;
        case t_const: puts("CONST"); break;
        case t_bool: puts("BOOL"); break;
        case t_char: puts("CHAR"); break;
        case t_string: puts("STRING"); break;
        case t_i8: puts("I8"); break;
        case t_u8: puts("U8"); break;
        case t_i16: puts("I16"); break;
        case t_u16: puts("U16"); break;
        case t_i32: puts("I32"); break;
        case t_u32: puts("U32"); break;
        case t_i64: puts("I64"); break;
        case t_u64: puts("U64"); break;
        case t_if: puts("IF"); break;
        case t_elif: puts("ELIF"); break;
        case t_else: puts("ELSE"); break;
        case t_while: puts("WHILE"); break;
        case t_is: puts("IS"); break;
        case t_then: puts("THEN"); break;
        case t_do: puts("DO"); break;
        case t_break: puts("BREAK"); break;
        case t_continue: puts("CONTINUE"); break;
        case t_import: puts("IMPORT"); break;
        case t_true: puts("TRUE"); break;
        case t_false: puts("FALSE"); break;
        case t_lgand: puts("AND"); break;
        case t_lgor: puts("OR"); break;
        
        case t_dot: puts("."); break;
        case t_semicolon: puts(";"); break;
        case t_comma: puts(","); break;
        case t_lparen: puts("("); break;
        case t_rparen: puts(")"); break;
        case t_lbracket: puts("["); break;
        case t_rbracket: puts("]"); break;
        case t_plus: puts("+"); break;
        case t_minus: puts("-"); break;
        case t_mul: puts("*"); break;
        case t_div: puts("/"); break;
        case t_mod: puts("%"); break;
        case t_and: puts("AND"); break;
        case t_or: puts("OR"); break;
        case t_xor: puts("XOR"); break;
        case t_colon: puts(":"); break;
        case t_gt: puts(">"); break;
        case t_gte: puts(">="); break;
        case t_lt: puts("<"); break;
        case t_lte: puts("<="); break;
        case t_eq: puts("="); break;
        case t_neq: puts("!="); break;
        case t_assign: puts(":="); break;
        case t_arrow: puts("->"); break;
        
        case t_id: printf("ID: %s\n", lex_get_id(l)); break;
        case t_int_literal: printf("INT_LITERAL: %d\n", lex_get_int(l)); break;
        case t_string_literal: printf("STR(%s)\n", lex_get_id(l)); break;
        case t_char_literal: printf("CHAR(%c)\n", lex_get_int(l)); break;
        
        default: puts("???");
    }
}

void lex_close(lex *l)
{
    free(l);
}


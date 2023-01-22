#pragma once

typedef enum {
    t_eof,
    t_none,
    
    t_extern,
    t_func,
    t_struct,
    t_end,
    t_return,
    t_var,
    t_const,
    t_bool, t_char, t_string,
    t_i8, t_u8, t_i16, t_u16,
    t_i32, t_u32, t_i64, t_u64,
    t_if, t_elif, t_else, t_while,
    t_is, t_then, t_do,
    t_break, t_continue,
    t_import,
    t_true, t_false,
    t_lgand, t_lgor,
    
    t_dot, t_semicolon, t_comma,
    t_lparen, t_rparen, t_lbracket, t_rbracket,
    t_plus, t_minus, t_mul, t_div, t_mod,
    t_and, t_or, t_xor,
    t_colon,
    t_gt, t_gte,
    t_lt, t_lte,
    t_eq, t_neq,
    t_assign,
    t_arrow,
    
    t_id,
    t_int_literal
} token;

typedef struct {
    char *input;
    char buffer[1024];
    int index;
    int buf_index;
    
    char last_buf[1024];
    int last_buf_len;
    
    token token_stack[10];
    int token_stack_len;
} lex;

lex *lex_init_string(char *input);
void lex_set_last_buffer(lex *l);
void lex_clear_buffer(lex *l);
int lex_is_symbol(char c);
token lex_get_symbol(lex *l, char c);
int lex_is_int(lex *l);
int lex_get_int(lex *l);
char *lex_get_id(lex *l);
void lex_rewind(lex *l, token tk);
token lex_get_next(lex *l);
void lex_debug(token t, lex *l);
void lex_close(lex *l);


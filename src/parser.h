#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include <stdbool.h>

typedef enum { LIST, ATOM, NUMBER, BOOLEAN, LAMBDA, NIL } exprtype_t;

typedef struct lambda_t {
  // Function arguments (all atoms)
  struct listitem_t *args;
  // Quoted body expression
  struct expr_t *body;
} lambda_t;

// list, atom, number, etc...
typedef struct expr_t {
  union {
    struct listitem_t *list;
    char atom[100];
    float number;
    bool boolean;
    lambda_t lambda;
  } value;
  exprtype_t type;
} expr_t;

typedef struct listitem_t {
  expr_t *expr;
  struct listitem_t *next;
} listitem_t;

listitem_t *list_last(listitem_t *item);

int list_length(listitem_t *item);

expr_t *new_expr();

expr_t *parse(token_t *tokens, int token_count, int start, int *walked);

expr_t *atom(token_t token);

void expr_to_string(expr_t *expr, char* buffer);

void print_expr(expr_t *expr);

void clone_listitem(listitem_t *dest, listitem_t *from);

void clone_expr(expr_t *dest, expr_t *from);

void free_listitem(listitem_t *item);

void free_expr(expr_t *expr);

#endif

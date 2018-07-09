#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LISTSIZE 100

listitem_t *list_last(listitem_t *item) {
  if (item == NULL)
    return NULL;

  listitem_t *last = item;
  while (last->next != NULL)
    last = last->next;

  return last;
}

int list_length(listitem_t *item) {
  if (item == NULL)
    return 0;

  int length = 1;

  listitem_t *last = item;
  while (last->next != NULL) {
    length++;
    last = last->next;
  }

  return length;
}

expr_t *new_expr() {
  expr_t *expr = malloc(sizeof(expr_t));
  expr->type = NIL;
  expr->value.list = NULL;
  return expr;
}

expr_t *parse(token_t *tokens, int token_count, int start, int *walked) {
  if (token_count == 0) {
    // Unexpected EOF
    return NULL;
  }

  expr_t *result;
  int i = start;
  listitem_t *item = NULL;
  int w;

  switch (tokens[i].type) {
  case OPEN_PARENS:
    result = new_expr();
    result->type = LIST;

    result->value.list = NULL;

    i++;
    while (tokens[i].type != CLOSE_PARENS && i < token_count) {
      item = malloc(sizeof(listitem_t));
      item->next = NULL;
      item->expr = parse(tokens, token_count, i, &w);

      if (result->value.list == NULL) {
        // List is empty
        result->value.list = item;
      } else {
        list_last(result->value.list)->next = item;
      }

      i++;
      i += w - 1;
    }
    if (tokens[i++].type != CLOSE_PARENS) {
      free_expr(result);
      return NULL;
    }
    if (walked != NULL)
      *walked = i - start;
    return result;
    break;
  case CLOSE_PARENS:
    // Unexpected CLOSE_PAREN
    if (walked != NULL)
      *walked = 1;
    return NULL;
    break;
  case WORD:
    if (walked != NULL)
      *walked = 1;
    float num;
    if (sscanf(tokens[i].contents, "%f", &num) == 1) {
      expr_t *numexpr = new_expr();
      numexpr->type = NUMBER;
      numexpr->value.number = num;
      return numexpr;
    }
    return atom(tokens[i]);
    break;

  default:
    return NULL;
    break;
  }

  return NULL;
}

expr_t *atom(token_t token) {
  expr_t *result = new_expr();

  result->type = ATOM;
  strcpy(result->value.atom, token.contents);

  return result;
}

void expr_to_string(expr_t *expr, char* buffer) {
  if (expr == NULL)
    return;

  listitem_t *listnext = expr->value.list;
  listitem_t *argsnext = expr->value.lambda.args;
  switch (expr->type) {
  case LIST:
    sprintf(buffer + strlen(buffer), " ( ");

    while (listnext != NULL) {
      expr_to_string(listnext->expr, buffer);
      listnext = listnext->next;
    }

    sprintf(buffer + strlen(buffer), " ) ");
    break;

  case ATOM:
    sprintf(buffer + strlen(buffer), " %s ", expr->value.atom);
    break;

  case NUMBER:
    sprintf(buffer + strlen(buffer), " %f ", expr->value.number);
    break;

  case NIL:
    sprintf(buffer + strlen(buffer), " nil ");
    break;

  case BOOLEAN:
    if (expr->value.boolean) {
      sprintf(buffer + strlen(buffer), " true ");
    } else {
      sprintf(buffer + strlen(buffer), " false ");
    }
    break;

  case LAMBDA:
    sprintf(buffer + strlen(buffer), " (  lambda ");

    sprintf(buffer + strlen(buffer), " ( ");

    while (argsnext != NULL) {
      expr_to_string(argsnext->expr, buffer);
      argsnext = argsnext->next;
    }

    sprintf(buffer + strlen(buffer), " ) ");

    expr_to_string(expr->value.lambda.body, buffer);

    sprintf(buffer + strlen(buffer), " ) ");

    break;
  }
}

void print_expr(expr_t *expr) {
  char buffer[5000] = "";
  expr_to_string(expr, buffer);
  buffer[strlen(buffer)-1] = '\0';
  printf("%s\n", &buffer[1]);
}

void clone_listitem(listitem_t *dest, listitem_t *from) {
  listitem_t *destitem = dest;
  listitem_t *item = from;
  while (item != NULL) {
    destitem->expr = new_expr();
    destitem->next = NULL;
    clone_expr(destitem->expr, item->expr);
    item = item->next;
    if (item != NULL) {
      destitem->next = malloc(sizeof(listitem_t));
      destitem = destitem->next;
    }
  }
}

void clone_expr(expr_t *dest, expr_t *from) {
  if (dest == NULL)
    return;

  dest->type = from->type;

  switch (from->type) {
  case LIST:
    if (list_length(from->value.list) > 0) {
      dest->value.list = malloc(sizeof(listitem_t));
      clone_listitem(dest->value.list, from->value.list);
    } else {
      dest->value.list = NULL;
    }
    break;

  case ATOM:
    strcpy(dest->value.atom, from->value.atom);
    break;

  case NUMBER:
    dest->value.number = from->value.number;
    break;

  case NIL:
    break;

  case BOOLEAN:
    dest->value.boolean = from->value.boolean;
    break;

  case LAMBDA:
    if (list_length(from->value.lambda.args) > 0) {
      dest->value.lambda.args = malloc(sizeof(listitem_t));
      clone_listitem(dest->value.lambda.args, from->value.lambda.args);
    } else {
      dest->value.lambda.args = NULL;
    }

    dest->value.lambda.body = new_expr();
    clone_expr(dest->value.lambda.body, from->value.lambda.body);

    break;
  }
}

void free_listitem(listitem_t *item) {
  if (item == NULL)
    return;

  if (item->next != NULL)
    free_listitem(item->next);

  free_expr(item->expr);
  free(item);
}

void free_expr(expr_t *expr) {
  if (expr == NULL)
    return;

  switch (expr->type) {
  case LIST:
    free_listitem(expr->value.list);
    break;
  case LAMBDA:
    free_listitem(expr->value.lambda.args);
    free_expr(expr->value.lambda.body);
    break;
  default:
    break;
  }

  free(expr);
}

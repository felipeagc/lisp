#include "eval.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void eval(expr_t* dest, expr_t *expr, env_t *env) {
  listitem_t *item = expr->value.list;

  switch (expr->type) {
  case NUMBER:
    clone_expr(dest, expr);
    break;

  case ATOM:
    if (strcmp(expr->value.atom, "nil") == 0) {
      dest->type = NIL;
      break;
    }

    if (strcmp(expr->value.atom, "true") == 0) {
      dest->type = BOOLEAN;
      dest->value.boolean = true;
      break;
    }

    if (strcmp(expr->value.atom, "false") == 0) {
      dest->type = BOOLEAN;
      dest->value.boolean = false;
      break;
    }

    var_t *var = env_find_var(env, expr->value.atom);
    if (var != NULL) {
      eval(dest, var->expr, env);
      break;
    }

    clone_expr(dest, expr);
    break;

  case LIST:
    if (list_length(item) > 0 && item->expr->type == ATOM) {
      fn_t *fn = env_find_fn(env, item->expr->value.atom);
      if (fn != NULL) {
        fn->callback(env, item->next, dest);
        break;
      }

      var_t *var = env_find_var(env, item->expr->value.atom);
      if (var != NULL && var->expr->type == LAMBDA) {
        env_t new_env;
        clone_env(&new_env, env);

        listitem_t *larg = var->expr->value.lambda.args;
        listitem_t *parg = item->next;

        if (list_length(larg) != list_length(parg)) {
          dest->type = NIL;
          break;
        }

        while (larg != NULL && parg != NULL) {
          if (larg->expr->type == ATOM) {
            expr_t *pexpr = new_expr();
            
            eval(pexpr, parg->expr, env);
            env_set_var(&new_env, larg->expr->value.atom, pexpr);

            free_expr(pexpr);
          }
          larg = larg->next;
          parg = parg->next;
        }

        eval(dest, var->expr->value.lambda.body, &new_env);

        free_env(new_env);
        break;
      }
    }

    dest->type = NIL;

    break;

  default:
    break;
  }
}

bool is_expr_true(expr_t expr) {
  switch (expr.type) {
  case NUMBER:
    return (expr.value.number != 0.0);
    break;

  case ATOM:
    return !(strcmp(expr.value.atom, "nil") == 0);
    break;

  case LIST:
    return (expr.value.list != NULL);
    break;

  case NIL:
    return false;
    break;

  case BOOLEAN:
    return expr.value.boolean;
    break;

  case LAMBDA:
    return true;
    break;
  }

  return false;
}

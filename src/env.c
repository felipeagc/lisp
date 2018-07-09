#include "env.h"
#include "eval.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

void add_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NUMBER;
  result->value.number = 0;

  listitem_t *last = args;
  expr_t *evaluated = new_expr();
  while (last != NULL) {
    eval(evaluated, last->expr, env);
    if (evaluated->type == NUMBER) {
      result->value.number += evaluated->value.number;
    }
    last = last->next;
  }
  free_expr(evaluated);
}

void sub_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NUMBER;

  if (list_length(args) == 0) {
    result->value.number = 0;
    return;
  } else {
    expr_t *first = new_expr();
    eval(first, args->expr, env);
    if (first->type == NUMBER) {
      result->value.number = first->value.number;
    }
    free_expr(first);
  }

  listitem_t *last = args->next;
  expr_t *evaluated = new_expr();
  while (last != NULL) {
    eval(evaluated, last->expr, env);
    if (evaluated->type == NUMBER) {
      result->value.number -= evaluated->value.number;
    }
    last = last->next;
  }
  free_expr(evaluated);
}

void mul_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NUMBER;
  if (list_length(args) == 0) {
    result->value.number = 0;
  } else {
    result->value.number = 1;
  }

  listitem_t *last = args;
  expr_t *evaluated = new_expr();
  while (last != NULL) {
    eval(evaluated, last->expr, env);
    if (evaluated->type == NUMBER) {
      result->value.number *= evaluated->value.number;
    }
    last = last->next;
  }
  free_expr(evaluated);
}

void div_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NUMBER;
  result->value.number = 0;

  bool first = true;

  listitem_t *last = args;
  expr_t *evaluated = new_expr();
  while (last != NULL) {
    eval(evaluated, last->expr, env);
    if (evaluated->type == NUMBER) {
      if (first) {
        result->value.number = evaluated->value.number;
        first = false;
      } else {
        result->value.number /= evaluated->value.number;
      }
    }
    last = last->next;
  }
  free_expr(evaluated);
}

void if_fn(env_t *env, listitem_t *args, expr_t *result) {
  // If must have at least 3 items
  // (if condition conseq)
  // (if condition conseq alt)
  // alt is optional
  if (list_length(args) < 2) {
    result->type = NIL;
    return;
  }

  expr_t *cond = new_expr();
  eval(cond, args->expr, env);

  if (is_expr_true(*cond)) {
    eval(result, args->next->expr, env);
  } else {
    if (args->next->next != NULL) {
      eval(result, args->next->next->expr, env);
    }
  }
  free_expr(cond);
}

void progn_fn(env_t *env, listitem_t *args, expr_t *result) {
  listitem_t *curr = args;
  while (curr != NULL) {
    eval(result, curr->expr, env);
    curr = curr->next;
  }
}

#define FN_CMP(name_fn, sign)                                                  \
  void name_fn(env_t *env, listitem_t *args, expr_t *result) {                 \
    if (list_length(args) != 2) {                                              \
      result->type = NIL;                                                      \
      return;                                                                  \
    }                                                                          \
    expr_t *eval1 = new_expr();                                                \
    expr_t *eval2 = new_expr();                                                \
    eval(eval1, args->expr, env);                                              \
    eval(eval2, args->next->expr, env);                                        \
    if (eval1->type == NUMBER && eval2->type == NUMBER) {                      \
      result->type = BOOLEAN;                                                  \
      result->value.boolean = eval1->value.number sign eval2->value.number;    \
      free_expr(eval1);                                                        \
      free_expr(eval2);                                                        \
      return;                                                                  \
    }                                                                          \
    free_expr(eval1);                                                          \
    free_expr(eval2);                                                          \
    result->type = NIL;                                                        \
  }

FN_CMP(lt_fn, <);
FN_CMP(lte_fn, <=);
FN_CMP(gt_fn, >);
FN_CMP(gte_fn, >=);
FN_CMP(equal_fn, ==);

void set_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NIL;

  if (list_length(args) == 2 && args->expr->type == ATOM) {
    expr_t *evaluated = new_expr();
    eval(evaluated, args->next->expr, env);
    env_set_var(env, args->expr->value.atom, evaluated);

    free_expr(evaluated);
  }
}

void quote_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NIL;

  if (list_length(args) >= 1) {
    clone_expr(result, args->expr);
  }
}

void print_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NIL;

  listitem_t *arg = args;
  while (arg != NULL) {
    expr_t* expr = new_expr();
    eval(expr, arg->expr, env);

    print_expr(expr);

    free_expr(expr);

    arg = arg->next;
  }
}

void fun_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NIL;

  if (list_length(args) == 2 && args->expr->type == LIST) {
    result->type = LAMBDA;

    if (list_length(args->expr->value.list) > 0) {
      result->value.lambda.args = malloc(sizeof(listitem_t));
      clone_listitem(result->value.lambda.args, args->expr->value.list);
    } else {
      result->value.lambda.args = NULL;
    }

    result->value.lambda.body = new_expr();
    clone_expr(result->value.lambda.body, args->next->expr);
  }
}

void defun_fn(env_t *env, listitem_t *args, expr_t *result) {
  // (lambda params body)
  // (defun a params body)

  result->type = NIL;

  if (list_length(args) != 3) {
    return;
  }

  listitem_t * name = args;
  listitem_t * largs = args->next;
  listitem_t * lbody = args->next->next;

  expr_t *lambda = new_expr();
  lambda->type = LAMBDA;

  if (list_length(largs->expr->value.list) > 0) {
    lambda->value.lambda.args = malloc(sizeof(listitem_t));
    clone_listitem(lambda->value.lambda.args, largs->expr->value.list);
  } else {
    lambda->value.lambda.args = NULL;
  }

  lambda->value.lambda.body = new_expr();
  clone_expr(lambda->value.lambda.body, lbody->expr);

  env_set_var(env, name->expr->value.atom, lambda);

  free_expr(lambda);
}

void not_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = BOOLEAN;
  result->value.boolean = false;

  if (list_length(args) == 1) {
    expr_t *expr = new_expr();
    eval(expr, args->expr, env);
    result->value.boolean = !is_expr_true(*expr);
    free_expr(expr);
  }
}

#define TRIG_FN(trig_fn, trig_op)                                         \
  void trig_fn(env_t *env, listitem_t *args, expr_t *result) { \
    result->type = NIL; \
    if (list_length(args) == 1) { \
      expr_t *expr = new_expr(); \
      eval(expr, args->expr, env); \
      if (expr->type == NUMBER) { \
        result->type = NUMBER; \
        result->value.number = trig_op(expr->value.number); \
      } \
      free_expr(expr); \
    } \
  }

TRIG_FN(sin_fn, sin);
TRIG_FN(cos_fn, cos);
TRIG_FN(tan_fn, tan);

void deg_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NIL;
  if (list_length(args) == 1) {
    expr_t *expr = new_expr();
    eval(expr, args->expr, env);
    if (expr->type == NUMBER) {
      result->type = NUMBER;
      result->value.number = expr->value.number / (M_PI / 180.0);
    }
    free_expr(expr);
  }
}

void rad_fn(env_t *env, listitem_t *args, expr_t *result) {
  result->type = NIL;
  if (list_length(args) == 1) {
    expr_t *expr = new_expr();
    eval(expr, args->expr, env);
    if (expr->type == NUMBER) {
      result->type = NUMBER;
      result->value.number = expr->value.number * M_PI / 180.0;
    }
    free_expr(expr);
  }
}

env_t new_std_env() {
  env_t env;
  env.functions = NULL;
  env.variables = NULL;

  // length
  env_add_fn(&env, "+", add_fn);
  env_add_fn(&env, "-", sub_fn);
  env_add_fn(&env, "*", mul_fn);
  env_add_fn(&env, "/", div_fn);

  env_add_fn(&env, "<", lt_fn);
  env_add_fn(&env, "<=", lte_fn);
  env_add_fn(&env, ">", gt_fn);
  env_add_fn(&env, ">=", gte_fn);
  env_add_fn(&env, "=", equal_fn);

  env_add_fn(&env, "if", if_fn);
  env_add_fn(&env, "progn", progn_fn);
  env_add_fn(&env, "set", set_fn);
  env_add_fn(&env, "quote", quote_fn);
  env_add_fn(&env, "fun", fun_fn);
  env_add_fn(&env, "print", print_fn);
  env_add_fn(&env, "defun", defun_fn);
  env_add_fn(&env, "not", not_fn);

  env_add_fn(&env, "sin", sin_fn);
  env_add_fn(&env, "cos", cos_fn);
  env_add_fn(&env, "tan", tan_fn);

  env_add_fn(&env, "deg", deg_fn);
  env_add_fn(&env, "rad", rad_fn);

  return env;
}

void env_set_var(env_t *env, char name[100], expr_t *value) {
  var_t *last_var = env->variables;
  if (last_var != NULL) {
    while (last_var->next != NULL) {
      last_var = last_var->next;
    }
  }

  if (last_var == NULL) {
    // Add at the root
    env->variables = malloc(sizeof(var_t));
    env->variables->next = NULL;
    env->variables->expr = new_expr();
    clone_expr(env->variables->expr, value);
    strcpy(env->variables->name, name);
  } else {
    var_t *var = env_find_var(env, name);
    if (var == NULL) {
      // Add at tail
      last_var->next = malloc(sizeof(var_t));

      last_var->next->expr = new_expr();
      last_var->next->next = NULL;

      clone_expr(last_var->next->expr, value);
      strcpy(last_var->next->name, name);
    } else {
      // Modify
      clone_expr(var->expr, value);
    }
  }
}

var_t *env_find_var(env_t *env, char name[100]) {
  var_t *var = env->variables;
  while (var != NULL) {
    if (strcmp(var->name, name) == 0) {
      return var;
    }
    var = var->next;
  }
  return NULL;
}

void env_add_fn(env_t *env, char name[100], fncallback_t callback) {
  fn_t *new = NULL;
  if (env->functions == NULL) {
    env->functions = malloc(sizeof(fn_t));
    new = env->functions;
  } else {
    new = env->functions;
    while (new->next != NULL)
      new = new->next;
    new->next = malloc(sizeof(fn_t));
    new = new->next;
  }

  strcpy(new->name, name);
  new->callback = callback;
  new->next = NULL;
}

fn_t *env_find_fn(env_t *env, char name[100]) {
  fn_t *fn = env->functions;
  while (fn != NULL) {
    if (strcmp(fn->name, name) == 0) {
      return fn;
    }
    fn = fn->next;
  }

  return NULL;
}

int env_fn_count(env_t *env) {
  if (env->functions == NULL)
    return 0;

  int length = 1;

  fn_t *last = env->functions;
  while (last->next != NULL) {
    length++;
    last = last->next;
  }

  return length;
}

int env_var_count(env_t *env) {
  if (env->variables == NULL)
    return 0;

  int length = 1;

  var_t *last = env->variables;
  while (last->next != NULL) {
    length++;
    last = last->next;
  }

  return length;
}

void clone_fn(fn_t *dest, fn_t *from) {
  fn_t *destfn = dest;
  fn_t *fn = from;
  while (fn != NULL) {
    destfn->callback = fn->callback;
    strcpy(destfn->name, fn->name);
    fn = fn->next;
    if (fn != NULL) {
      destfn->next = malloc(sizeof(fn_t));
      destfn->next->next = NULL;
      destfn = destfn->next;
    }
  }
}

void clone_var(var_t *dest, var_t *from) {
  var_t *destvar = dest;
  var_t *var = from;
  while (var != NULL) {
    clone_expr(destvar->expr, var->expr);
    strcpy(destvar->name, var->name);
    var = var->next;
    if (var != NULL) {
      destvar->next = malloc(sizeof(var_t));
      destvar->next->expr = new_expr();
      destvar->next->next = NULL;
      destvar = destvar->next;
    }
  }
}

void clone_env(env_t *dest, env_t *from) {
  dest->functions = NULL;
  dest->variables = NULL;

  if (env_fn_count(from) > 0) {
    dest->functions = malloc(sizeof(fn_t));
    dest->functions->next = NULL;
    clone_fn(dest->functions, from->functions);
  }

  if (env_var_count(from) > 0) {
    dest->variables = malloc(sizeof(var_t));
    dest->variables->expr = new_expr();
    dest->variables->next = NULL;
    clone_var(dest->variables, from->variables);
  }
}

void free_fn(fn_t *fn) {
  if (fn == NULL)
    return;

  if (fn->next != NULL)
    free_fn(fn->next);

  free(fn);
}

void free_var(var_t *var) {
  if (var == NULL)
    return;

  if (var->next != NULL)
    free_var(var->next);

  free_expr(var->expr);
  free(var);
}

void free_env(env_t env) {
  free_fn(env.functions);
  free_var(env.variables);
}

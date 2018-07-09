#ifndef ENV_H
#define ENV_H

#include "parser.h"

typedef struct env_t {
  struct fn_t *functions;
  struct var_t *variables;
} env_t;

typedef void (*fncallback_t)(env_t *, listitem_t *, expr_t *);

typedef struct fn_t {
  char name[100];
  fncallback_t callback;
  struct fn_t *next;
} fn_t;

typedef struct var_t {
  char name[100];
  expr_t* expr;
  struct var_t *next;
} var_t;

env_t new_std_env();

void env_set_var(env_t *env, char name[100], expr_t *value);

var_t* env_find_var(env_t *env, char name[100]);

void env_add_fn(env_t *env, char name[100], fncallback_t callback);

fn_t *env_find_fn(env_t *env, char name[100]);

void clone_env(env_t *dest, env_t *from);

void free_env(env_t env);

#endif

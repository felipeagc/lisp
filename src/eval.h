#ifndef EVAL_H
#define EVAL_H

#include <stdbool.h>
#include "parser.h"
#include "env.h"

void eval(expr_t *dest, expr_t *expr, env_t *env);

bool is_expr_true(expr_t expr);

#endif

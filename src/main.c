#include "env.h"
#include "eval.h"
#include "parser.h"
#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int testn = 1;

void test(char *input) {
  int token_count;
  token_t *tokens = tokenize(input, &token_count);

  printf("TEST %d\n", testn);
  printf(" Input: \"%s\"\n", input);

  expr_t *parsed = parse(tokens, token_count, 0, NULL);

  env_t env = new_std_env();

  if (parsed == NULL) {
    // Parsing error
    printf(" Parsing error\n\n");
  } else {
    printf(" Parsed: ");

    print_expr(parsed);

    expr_t *evaluated = new_expr();
    eval(evaluated, parsed, &env);

    printf(" Evaluated: ");

    print_expr(evaluated);

    printf("Of type: %d", evaluated->type);
    printf("\n\n");

    free_expr(evaluated);
  }

  free_env(env);
  free_expr(parsed);
  free(tokens);

  testn++;
}

void run_tests() {
  test("");
  test("(");
  test(")");
  test("(1 2");

  test("()");
  test("(abc def)");
  test("(1 2 3)");
  test("(quote (1 (+ 2 3) 3))");
  test("(quote (abc def))");
  test("123");

  test("(+)");
  test("(+ 1)");
  test("(+ 1 2 3)");
  test("(- 2 1)");
  test("(* 3 4 (/ 1 2))");
  test("(/ 3 4 5)");
  test("(if 1 2 (+ 1 2))");
  test("(if 0 2 (+ 1 2))");
  test("(progn (if 1 (+ 1 4) 3) -5.3 4.3)");
  test("(< 1 (+ 1 2))");
  test("(< (+ 3 4) 2)");
  test("(= (* 2 3) 6)");

  test("(progn (set a 123) a)");
  test("(progn (set a 123) (set a 321) a)");
  test("(progn (set a 123) (if (< a 12) 1 0))");
  test("(progn (set a 1) (set b 2) (< a b))");
  test("(progn (set a 1) (set b 2) (> a b))");
  test("(progn (set a 1) (set b 2) (= a b))");

  test("(fun (a b) (> a b))");
  test("(progn (set gt (fun (a b) (< a b)))"
       "       (gt 1 2)"
       ")");
  test("(progn (defun gt (a b) (< a b))"
       "       (gt 1 2)"
       ")");
  test("(> 1 2)");
  test("(> 1 b)");

  test("(progn (defun fat (a) (if (= a 1) 1 (* a (fat (- a 1)))))"
       "       (fat 5))");

  test("(progn (defun sum (a) (if (= a 1) 1 (+ a (sum (- a 1)))))"
       "       (sum 4))");

}

void run_repl() {
  env_t env = new_std_env();

  while (1) {
    char input[1000];
    printf("=> ");
    fgets(input, 1000, stdin);

    int token_count;
    token_t *tokens = tokenize(input, &token_count);

    expr_t *parsed = parse(tokens, token_count, 0, NULL);

    if (parsed == NULL) {
      // Parsing error
      printf(" Parsing error.\n\n");
    } else {
      expr_t *evaluated = new_expr();
      eval(evaluated, parsed, &env);

      print_expr(evaluated);
      printf("\n");

      free_expr(evaluated);
    }

    free_expr(parsed);
    free(tokens);
  }

  free_env(env);
}

char* load_file(char* filename) {
  char *buffer = NULL;
  long int length;
  FILE* f = fopen(filename, "rb");
  if (f != NULL) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length);
    if (buffer != NULL) {
      fread(buffer, 1, length, f);
    }

    fclose(f);
  }

  return buffer;
}

void run_file(char* filename) {
  env_t env = new_std_env();

  char* input = load_file(filename);
  if (input == NULL) {
    printf("Error reading file: \"%s\"\n", filename);
    return;
  }

  int token_count;
  token_t *tokens = tokenize(input, &token_count);

  expr_t *parsed = parse(tokens, token_count, 0, NULL);

  if (parsed == NULL) {
    // Parsing error
    printf(" Parsing error.\n\n");
  } else {
    expr_t *evaluated = new_expr();
    eval(evaluated, parsed, &env);

    free_expr(evaluated);
  }

  free_expr(parsed);
  free(tokens);

  free_env(env);
}

int main(int argc, char **argv) {
  if (argc == 1) {
    // Repl
    run_repl();

    return 0;
  } else if (argc == 2) {
    if (strcmp(argv[1], "--tests") == 0 || strcmp(argv[1], "-t") == 0) {
      // Run tests
      run_tests();
      return 0;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      // Display help
      return 0;
    }

    // Load file
    run_file(argv[1]);
  }

  return 0;
}

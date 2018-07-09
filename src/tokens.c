#include "tokens.h"
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 100

#define FINALIZE_WORD()                                                        \
  if (strlen(buffer) > 0) {                                                    \
    ADD_TOKEN(WORD, buffer);                                                   \
    empty_str(buffer, BUFSIZE);                                                \
    bufi = 0;                                                                  \
  }

#define ADD_TOKEN(ttype, tcontents)                                            \
  if (token_max <= toki) {                                                     \
    token_max *= 2;                                                            \
    tokens = realloc(tokens, sizeof(token_t) * token_max);                     \
    set_blank_tokens(tokens, toki, token_max);                                 \
  }                                                                            \
  tokens[toki].type = ttype;                                                   \
  strcpy(tokens[toki].contents, tcontents);                                    \
  toki++;

void empty_str(char *str, int n) {
  int i;
  for (i = 0; i < n; i++) {
    str[i] = '\0';
  }
}

void set_blank_tokens(token_t *tokens, int start, int token_max) {
  int i;
  for (i = start; i < token_max; i++) {
    tokens[i].type = BLANK;
  }
}

token_t *tokenize(const char *str, int *token_count) {
  int token_max = 10;
  token_t *tokens = malloc(sizeof(token_t) * token_max);
  char buffer[BUFSIZE] = "";

  int toki = 0;
  int bufi = 0;

  set_blank_tokens(tokens, toki, token_max);

  int i;
  for (i = 0; i < strlen(str); i++) {
    switch (str[i]) {
    case '\n':
    case ' ':
      FINALIZE_WORD();
      break;

    case '(':
      FINALIZE_WORD();
      ADD_TOKEN(OPEN_PARENS, "(");
      break;

    case ')':
      FINALIZE_WORD();
      ADD_TOKEN(CLOSE_PARENS, ")");
      break;

    default:
      buffer[bufi++] = str[i];
      break;
    }
  }

  FINALIZE_WORD();

  *token_count = toki;
  return tokens;
}

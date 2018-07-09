#ifndef TOKENS_H
#define TOKENS_H

#define TOKEN_BUFSIZE 100

typedef enum { BLANK, OPEN_PARENS, CLOSE_PARENS, WORD } tokentype_t;

typedef struct {
  char contents[TOKEN_BUFSIZE];
  tokentype_t type;
} token_t;

void empty_str(char *str, int n);

token_t *tokenize(const char *str, int *token_count);

#endif

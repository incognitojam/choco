#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum TokenType {
  TOKEN_VAR,
  TOKEN_IDENTIFIER,
  TOKEN_EQUALS,
  TOKEN_NUMBER,
  TOKEN_STRING,
  TOKEN_SEMICOLON,
} TokenType;

typedef struct Token {
  TokenType type;
  union {
    struct {
      char *name;
    } identifier;
    struct {
      double value;
    } number;
    struct {
      char *value;
    } string;
  };
} Token;

Token *createToken(TokenType type) {
  Token *token = (Token *)malloc(sizeof(Token));
  token->type = type;
  return token;
}

typedef struct TokenNode {
  Token *token;
  struct TokenNode *next;
} TokenNode;

typedef struct TokenList {
  TokenNode *head;
  TokenNode *tail;
} TokenList;

TokenList *createTokenList() {
  TokenList *list = (TokenList *)malloc(sizeof(TokenList));
  list->head = NULL;
  return list;
}

void addToken(TokenList *list, Token *token) {
  TokenNode *node = (TokenNode *)malloc(sizeof(TokenNode));
  node->token = token;
  node->next = NULL;
  if (list->head == NULL) {
    list->head = node;
  } else {
    list->tail->next = node;
  }
  list->tail = node;
}

void dumpTokenList(TokenList *list) {
  TokenNode *node = list->head;
  while (node != NULL) {
    Token *token = node->token;
    switch (token->type) {
    case TOKEN_VAR:
      printf("VAR ");
      break;
    case TOKEN_IDENTIFIER:
      printf("IDENTIFIER %s ", token->identifier.name);
      break;
    case TOKEN_EQUALS:
      printf("EQUALS ");
      break;
    case TOKEN_NUMBER:
      printf("NUMBER %f ", token->number.value);
      break;
    case TOKEN_STRING:
      printf("STRING %s ", token->string.value);
      break;
    case TOKEN_SEMICOLON:
      printf("SEMICOLON ");
      break;
    }
    node = node->next;
  }
  printf("\n");
}

void freeTokenList(TokenList *list) {
  TokenNode *node = list->head;
  while (node != NULL) {
    TokenNode *next = node->next;
    free(node->token->identifier.name);
    free(node->token);
    free(node);
    node = next;
  }
  free(list);
}

char *extract_while(char **current_ptr, int (*condition)(int)) {
  char *start = *current_ptr;
  char *end = start;
  while (*end && condition(*end)) {
    end++;
  }

  int length = end - start;
  if (length == 0) {
    return NULL;
  }

  char *result = (char *)malloc(length + 1);
  if (!result) {
    return NULL;
  }
  memcpy(result, start, length);
  result[length] = '\0';

  *current_ptr = end;
  return result;
}

int is_identifier_char(int c) { return isalpha(c) || isdigit(c) || c == '_'; }

int is_digit_char(int c) { return isdigit(c); }

void tokenizeFile(FILE *input, TokenList *list) {
  int c;
  while ((c = fgetc(input)) != EOF) {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      // skip whitespace
      continue;
    }

    if (c == ';') {
      addToken(list, createToken(TOKEN_SEMICOLON));
      continue;
    }

    if (c == '=') {
      addToken(list, createToken(TOKEN_EQUALS));
      continue;
    }

    if (isdigit(c)) {
      ungetc(c, input);
      char buffer[256];
      int i = 0;
      bool hasDecimal = false;
      while ((c = fgetc(input)) != EOF &&
             (isdigit(c) || (!hasDecimal && c == '.'))) {
        if (c == '.') {
          hasDecimal = true;
        }
        buffer[i++] = c;
      }
      buffer[i] = '\0';
      if (c != EOF) {
        ungetc(c, input);
      }

      Token *token = createToken(TOKEN_NUMBER);
      token->number.value = atof(buffer);
      addToken(list, token);
      continue;
    }

    if (isalpha(c)) {
      ungetc(c, input);
      char buffer[256];
      int i = 0;
      while ((c = fgetc(input)) != EOF &&
             (isalpha(c) || isdigit(c) || c == '_')) {
        buffer[i++] = c;
      }
      buffer[i] = '\0';
      if (c != EOF) {
        ungetc(c, input);
      }

      char *name = (char *)malloc(strlen(buffer) + 1);
      strcpy(name, buffer);

      Token *token;
      if (strcmp("var", name) == 0) {
        token = createToken(TOKEN_VAR);
        free(name);
      } else {
        token = createToken(TOKEN_IDENTIFIER);
        token->identifier.name = name;
      }
      addToken(list, token);
      continue;
    }

    if (c == '"') {
      char buffer[1024];
      int i = 0;
      while ((c = fgetc(input)) != EOF && c != '"') {
        buffer[i++] = c;
      }
      buffer[i] = '\0';

      char *value = (char *)malloc(strlen(buffer) + 1);
      strcpy(value, buffer);

      Token *token = createToken(TOKEN_STRING);
      token->string.value = value;
      addToken(list, token);
      continue;
    }

    assert(false);
  }
}

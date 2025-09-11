#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum TokenType {
  TOKEN_UNKNOWN,
  TOKEN_VAR,
  TOKEN_IDENTIFIER,
  TOKEN_EQUALS,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_MULTIPLY,
  TOKEN_DIVIDE,
  TOKEN_PERIOD,
  TOKEN_LEFT_PARENS,
  TOKEN_RIGHT_PARENS,
  TOKEN_OPENING_BRACKET,
  TOKEN_CLOSING_BRACKET,
  TOKEN_OPENING_BRACE,
  TOKEN_CLOSING_BRACE,
  TOKEN_NUMBER,
  TOKEN_STRING,
  TOKEN_SEMICOLON,
} TokenType;

typedef struct SingleCharToken {
  char character;
  TokenType type;
  const char *name;
} SingleCharToken;

static const SingleCharToken SINGLE_CHAR_TOKENS[] = {
    {';', TOKEN_SEMICOLON, "SEMICOLON"},
    {'=', TOKEN_EQUALS, "EQUALS"},
    {'+', TOKEN_PLUS, "PLUS"},
    {'-', TOKEN_MINUS, "MINUS"},
    {'*', TOKEN_MULTIPLY, "MULTIPLY"},
    {'/', TOKEN_DIVIDE, "DIVIDE"},
    {'.', TOKEN_PERIOD, "PERIOD"},
    {'(', TOKEN_LEFT_PARENS, "LEFT PARENS"},
    {')', TOKEN_RIGHT_PARENS, "RIGHT PARENS"},
    {'[', TOKEN_OPENING_BRACKET, "OPENING BRACKET"},
    {']', TOKEN_CLOSING_BRACKET, "CLOSING BRACKET"},
    {'{', TOKEN_OPENING_BRACE, "OPENING BRACE"},
    {'}', TOKEN_CLOSING_BRACE, "CLOSING BRACE"},
    {0, TOKEN_UNKNOWN, NULL} // Sentinel value
};

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

TokenType findSingleCharToken(char c) {
  for (int i = 0; SINGLE_CHAR_TOKENS[i].character != 0; i++) {
    if (SINGLE_CHAR_TOKENS[i].character == c) {
      return SINGLE_CHAR_TOKENS[i].type;
    }
  }
  return TOKEN_UNKNOWN;
}

const char *getSingleCharTokenName(TokenType type) {
  for (int i = 0; SINGLE_CHAR_TOKENS[i].character != 0; i++) {
    if (SINGLE_CHAR_TOKENS[i].type == type) {
      return SINGLE_CHAR_TOKENS[i].name;
    }
  }
  return NULL;
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
    case TOKEN_NUMBER:
      printf("NUMBER %f ", token->number.value);
      break;
    case TOKEN_STRING:
      printf("STRING %s ", token->string.value);
      break;
    default: {
      const char *name = getSingleCharTokenName(token->type);
      if (name != NULL) {
        printf("%s ", name);
      } else {
        printf("UNKNOWN ");
      }
      break;
    }
    }
    node = node->next;
  }
  printf("\n");
}

void freeTokenList(TokenList *list) {
  TokenNode *node = list->head;
  while (node != NULL) {
    TokenNode *next = node->next;
    if (node->token->type == TOKEN_IDENTIFIER) {
      free(node->token->identifier.name);
    } else if (node->token->type == TOKEN_STRING) {
      free(node->token->string.value);
    }
    free(node->token);
    free(node);
    node = next;
  }
  free(list);
}

void tokenizeFile(FILE *input, TokenList *list) {
  int c;
  while ((c = fgetc(input)) != EOF) {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      // skip whitespace
      continue;
    }

    TokenType singleCharType = findSingleCharToken(c);
    if (singleCharType != TOKEN_UNKNOWN) {
      addToken(list, createToken(singleCharType));
      continue;
    }

    if (isdigit(c)) {
      ungetc(c, input);
      char buffer[256];
      int i = 0;
      bool hasDecimal = false;
      while ((c = fgetc(input)) != EOF && i < (sizeof(buffer) - 1) &&
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
      while ((c = fgetc(input)) != EOF && i < (sizeof(buffer) - 1) &&
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
      while ((c = fgetc(input)) != EOF && i < (sizeof(buffer) - 1) &&
             c != '"') {
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

    fprintf(stderr, "Unexpected character: %c\n", c);
    abort();
  }
}

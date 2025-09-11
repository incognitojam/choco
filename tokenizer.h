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

#include <stdio.h>

#include "tokenizer.h"

int main() {
  TokenList *list = createTokenList();

  Token *var = createToken(TOKEN_VAR);
  addToken(list, var);

  Token *ident = createToken(TOKEN_IDENTIFIER);
  ident->identifier.name = strdup("x");
  addToken(list, ident);

  Token *eq = createToken(TOKEN_EQUALS);
  addToken(list, eq);

  Token *value = createToken(TOKEN_NUMBER);
  value->number.value = 0;
  addToken(list, value);

  Token *semicolon = createToken(TOKEN_SEMICOLON);
  addToken(list, semicolon);

  dumpTokenList(list);
  freeTokenList(list);
}

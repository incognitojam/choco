#include <stdio.h>

#include "tokenizer.h"

int main() {
  TokenList *toks = createTokenList();

  /*
  Token *var = createToken(TOKEN_VAR);
  addToken(toks, var);

  Token *ident = createToken(TOKEN_IDENTIFIER);
  ident->identifier.name = strdup("x");
  addToken(toks, ident);

  Token *eq = createToken(TOKEN_EQUALS);
  addToken(toks, eq);

  Token *value = createToken(TOKEN_NUMBER);
  value->number.value = 0;
  addToken(toks, value);

  Token *semicolon = createToken(TOKEN_SEMICOLON);
  addToken(toks, semicolon);
  */

  dumpTokenList(toks);
  freeTokenList(toks);
}

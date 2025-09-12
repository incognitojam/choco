#include <stdio.h>

#include "parser.h"

int main(int argc, char *argv[]) {
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

  const char *filename = "examples/vars.js"; // default
  if (argc > 1) {
    filename = argv[1];
  }

  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening file '%s': ", filename);
    perror("");
    return 1;
  }
  tokenizeFile(file, toks);
  fclose(file);

  printf("=== TOKENS ===\n");
  dumpTokenList(toks);

  printf("\n=== AST ===\n");
  ASTNode *ast = parseTokens(toks);
  if (ast != NULL) {
    dumpAST(ast, 0);
  } else {
    printf("Failed to parse\n");
  }

  freeTokenList(toks);
}

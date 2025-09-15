#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

typedef enum ASTNodeType {
  AST_PROGRAM,
  AST_VARIABLE_DECLARATION,
  AST_BINARY_EXPRESSION,
  AST_IDENTIFIER,
  AST_NUMBER,
} ASTNodeType;

typedef enum BinaryExpressionOperator {
  OP_PLUS,
  OP_MINUS,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_UNKNOWN,
} BinaryExpressionOperator;

const char *getOperatorName(BinaryExpressionOperator op) {
  static const char *names[] = {
      "PLUS",
      "MINUS",
      "MULTIPLY",
      "DIVIDE",
  };
  if (op >= 0 && op < OP_UNKNOWN) {
    return names[op];
  }
  return "UNKNOWN";
}

typedef struct ASTNode {
  ASTNodeType type;
  union {
    struct {
      struct ASTNode **statements;
      int statement_count;
    } program;
    struct {
      char *name;
      struct ASTNode *value;
    } variable_declaration;
    struct {
      BinaryExpressionOperator op;
      struct ASTNode *left;
      struct ASTNode *right;
    } binary_expression;
    struct {
      char *name;
    } identifier;
    struct {
      double value;
    } number;
  };
  struct ASTNode **children;
  int child_count;
} ASTNode;

typedef struct Parser {
  TokenNode *current;
} Parser;

ASTNode *parseExpression(Parser *parser);
ASTNode *parseBinaryExpression(Parser *parser, ASTNode *left, int minPrec);
ASTNode *parsePrimary(Parser *parser);

ASTNode *createASTNode(ASTNodeType type) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  node->type = type;
  node->children = NULL;
  node->child_count = 0;
  return node;
}

Parser *createParser(TokenList *tokens) {
  Parser *parser = (Parser *)malloc(sizeof(Parser));
  parser->current = tokens->head;
  return parser;
}

Token *peek(Parser *parser) {
  if (parser->current == NULL) {
    return NULL;
  }
  return parser->current->token;
}

Token *advance(Parser *parser) {
  if (parser->current == NULL) {
    return NULL;
  }
  Token *token = parser->current->token;
  parser->current = parser->current->next;
  return token;
}

bool match(Parser *parser, TokenType type) {
  Token *token = peek(parser);
  return token != NULL && token->type == type;
}

Token *consume(Parser *parser, TokenType type) {
  if (match(parser, type)) {
    return advance(parser);
  }
  fprintf(stderr, "Expected token type %d\n", type);
  return NULL;
}

ASTNode *parseNumber(Parser *parser) {
  Token *token = consume(parser, TOKEN_NUMBER);
  if (token == NULL)
    return NULL;

  ASTNode *node = createASTNode(AST_NUMBER);
  node->number.value = token->number.value;
  return node;
}

ASTNode *parseIdentifier(Parser *parser) {
  Token *token = consume(parser, TOKEN_IDENTIFIER);
  if (token == NULL)
    return NULL;

  ASTNode *node = createASTNode(AST_IDENTIFIER);
  node->identifier.name = strdup(token->identifier.name);
  return node;
}

ASTNode *parsePrimary(Parser *parser) {
  if (match(parser, TOKEN_NUMBER)) {
    return parseNumber(parser);
  }
  if (match(parser, TOKEN_IDENTIFIER)) {
    return parseIdentifier(parser);
  }
  if (match(parser, TOKEN_LEFT_PARENS)) {
    advance(parser); // consume '('
    ASTNode *expr = parseExpression(parser);
    consume(parser, TOKEN_RIGHT_PARENS);
    return expr;
  }
  return NULL;
}

ASTNode *parseBinaryExpression(Parser *parser, ASTNode *left, int minPrec) {
  while (true) {
    Token *op = peek(parser);
    if (op == NULL)
      break;

    int prec = 0;
    if (op->type == TOKEN_PLUS || op->type == TOKEN_MINUS) {
      prec = 1;
    } else if (op->type == TOKEN_MULTIPLY || op->type == TOKEN_DIVIDE) {
      prec = 2;
    } else {
      break;
    }

    if (prec < minPrec)
      break;

    advance(parser); // consume operator
    ASTNode *right = parsePrimary(parser);
    if (right == NULL)
      break;

    // Handle right associativity
    Token *nextOp = peek(parser);
    if (nextOp != NULL) {
      int nextPrec = 0;
      if (nextOp->type == TOKEN_PLUS || nextOp->type == TOKEN_MINUS) {
        nextPrec = 1;
      } else if (nextOp->type == TOKEN_MULTIPLY ||
                 nextOp->type == TOKEN_DIVIDE) {
        nextPrec = 2;
      }

      if (nextPrec > prec) {
        right = parseBinaryExpression(parser, right, nextPrec);
      }
    }

    ASTNode *node = createASTNode(AST_BINARY_EXPRESSION);
    node->binary_expression.left = left;
    node->binary_expression.right = right;

    switch (op->type) {
    case TOKEN_PLUS:
      node->binary_expression.op = OP_PLUS;
      break;
    case TOKEN_MINUS:
      node->binary_expression.op = OP_MINUS;
      break;
    case TOKEN_MULTIPLY:
      node->binary_expression.op = OP_MULTIPLY;
      break;
    case TOKEN_DIVIDE:
      node->binary_expression.op = OP_DIVIDE;
      break;
    default:
      node->binary_expression.op = OP_UNKNOWN;
      break;
    }

    left = node;
  }

  return left;
}

ASTNode *parseExpression(Parser *parser) {
  ASTNode *left = parsePrimary(parser);
  if (left == NULL)
    return NULL;

  return parseBinaryExpression(parser, left, 0);
}

ASTNode *parseVariableDeclaration(Parser *parser) {
  consume(parser, TOKEN_VAR);

  Token *nameToken = consume(parser, TOKEN_IDENTIFIER);
  if (nameToken == NULL)
    return NULL;

  consume(parser, TOKEN_EQUALS);

  ASTNode *value = parseExpression(parser);
  if (value == NULL)
    return NULL;

  consume(parser, TOKEN_SEMICOLON);

  ASTNode *node = createASTNode(AST_VARIABLE_DECLARATION);
  node->variable_declaration.name = strdup(nameToken->identifier.name);
  node->variable_declaration.value = value;

  return node;
}

ASTNode *parseStatement(Parser *parser) {
  if (match(parser, TOKEN_VAR)) {
    return parseVariableDeclaration(parser);
  }

  // For now, treat other things as expression statements
  ASTNode *expr = parseExpression(parser);
  if (expr != NULL) {
    consume(parser, TOKEN_SEMICOLON);
  }
  return expr;
}

void dumpAST(ASTNode *node, int indent) {
  if (node == NULL)
    return;

  for (int i = 0; i < indent; i++)
    printf("  ");

  switch (node->type) {
  case AST_PROGRAM:
    printf("PROGRAM\n");
    for (int i = 0; i < node->program.statement_count; i++) {
      dumpAST(node->program.statements[i], indent + 1);
    }
    break;
  case AST_VARIABLE_DECLARATION:
    printf("VAR_DECL: %s\n", node->variable_declaration.name);
    dumpAST(node->variable_declaration.value, indent + 1);
    break;
  case AST_BINARY_EXPRESSION:
    printf("BINARY_OP: %s\n", getOperatorName(node->binary_expression.op));
    dumpAST(node->binary_expression.left, indent + 1);
    dumpAST(node->binary_expression.right, indent + 1);
    break;
  case AST_IDENTIFIER:
    printf("IDENTIFIER: %s\n", node->identifier.name);
    break;
  case AST_NUMBER:
    printf("NUMBER: %f\n", node->number.value);
    break;
  }
}

ASTNode *parseTokens(TokenList *tokens) {
  Parser *parser = createParser(tokens);

  ASTNode *program = createASTNode(AST_PROGRAM);
  program->program.statements = NULL;
  program->program.statement_count = 0;

  while (peek(parser) != NULL) {
    ASTNode *stmt = parseStatement(parser);
    if (stmt == NULL)
      break;

    program->program.statement_count++;
    program->program.statements = (ASTNode **)realloc(
        program->program.statements,
        sizeof(ASTNode *) * program->program.statement_count);
    program->program.statements[program->program.statement_count - 1] = stmt;
  }

  free(parser);
  return program;
}

#endif // PARSER_H

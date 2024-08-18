#ifndef PARSER_H

#include<iostream>
#include<string>
#include<memory>
#include"lexer.h"
#include<vector>
#include"type.h"


struct Decl{
  int line;
  int col;
  std::string ident;

  Decl(int line, int col, std::string ident): ident(std::move(ident)) {
    this->line = line;
    this->col = col;
  }

  virtual ~Decl() = default;

  virtual void dump(std::size_t level = 0) const = 0;
};

struct Stmt{
  int line;
  int col;

  Stmt(int line, int col) {
    this->line = line;
    this->col = col;
  }

  virtual ~Stmt() = default;

  virtual void dump(std::size_t level = 0) const = 0;
};

struct Expr : public Stmt{
  Expr(int line, int col): Stmt(line, col){}
};

struct CallExpr: public Expr{
  std::unique_ptr<Expr> callee;
  std::vector<std::unique_ptr<Expr>> arguments;

  CallExpr(int line, int col, std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments):
    Expr(line, col),
    callee(std::move(callee)),
    arguments(std::move(arguments)){}
  
  void dump(std::size_t level = 0) const override;
};

struct DeclRefExpr: public Expr{
  std::string identifier;

  DeclRefExpr(int line, int col, std::string ident)
  : Expr(line, col),
    identifier(std::move(ident)){}
  
  void dump(std::size_t level = 0) const override;
};

struct NumberLiteral : public Expr{
  std::string value;

  NumberLiteral(int line, int col, std::string value): 
  Expr(line, col),
  value(std::move(value)){}

  void dump(std::size_t level = 0) const override;
};

struct Block{
  int line;
  int col;
  std::vector<std::unique_ptr<Stmt>> statements;

  Block(int line, int col, std::vector<std::unique_ptr<Stmt>> statements)
  : statements(std::move(statements)) {
    this->line = line;
    this->col = col;
  }

  void dump (std::size_t level) const;
};

struct ReturnStmt : public Stmt{
  std::unique_ptr<Expr> expr;
  
  ReturnStmt(int line, int col, std::unique_ptr<Expr> expr) : 
  Stmt(line, col), 
  expr(std::move(expr)){}

  void dump(std::size_t level = 0) const override;

};

struct ParamDecl: public Decl{
  Type type;

  ParamDecl(int line, int col, std::string ident, Type type)
  : Decl(line, col, std::move(ident)),
    type(std::move(type)){}
  
  void dump(std::size_t level = 0) const override;
};

struct FunctionDecl: public Decl{
  Type type;
  std::vector<std::unique_ptr<ParamDecl>> params;
  std::unique_ptr<Block> body;

  FunctionDecl(int line, int col, std::string ident, Type type, 
  std::vector<std::unique_ptr<ParamDecl>> params,
  std::unique_ptr<Block> body): 
   Decl(line, col, std::move(ident))
  ,params(std::move(params))
  ,body(std::move(body))
  ,type(std::move(type)){}

  void dump(std::size_t level = 0) const override;
};

class Parser{
  Lexer *lexer;
  Token nextToken;
  bool isIncompAST = false;
  public:
  explicit Parser(Lexer &lexer): lexer(&lexer), nextToken(lexer.getNextToken()){}
  void eatNextToken();
  std::pair<std::vector<std::unique_ptr<FunctionDecl>>, bool> 
  parseSourceFile();
  void synchronizeOn(TokenKind kind);
  std::unique_ptr<FunctionDecl> parseFunctionDecl();
  std::optional<Type> parseType();
  std::unique_ptr<Block> parseBlock();
  std::unique_ptr<ReturnStmt> parseReturnStmt();
  std::unique_ptr<Stmt> parseStmt();
  std::unique_ptr<Expr> parsePrimary();
  std::unique_ptr<Expr> parseExpr();
  std::unique_ptr<Expr> parsePostfixExpr();
  std::unique_ptr<std::vector<std::unique_ptr<Expr>>> parseArgumentList();
  std::unique_ptr<ParamDecl> parseParamDecl();
  std::unique_ptr<std::vector<std::unique_ptr<ParamDecl>>> parseParamList();
};

#endif
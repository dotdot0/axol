#include <iostream>
#include <string>
#include <vector>
#include "lexer.h"
#include "type.h"
#include <memory>

struct ResolvedStmt {
  int line;
  int col;

  ResolvedStmt(int line, int col) {
    this->line = line;
    this->col = col;
  }

  virtual ~ResolvedStmt() = default;

  virtual void dump(std::size_t level = 0) const = 0;
};

struct ResolvedExpr: public ResolvedStmt{
  Type type;

  ResolvedExpr(int line, int col, Type type)
  : ResolvedStmt(line, col),
    type(type){}
};

struct ResolvedDecl {
  int line;
  int col;
  std::string ident;
  Type type;

  ResolvedDecl(int line, int col, std::string ident, Type type)
  : line(line), col(col),
    ident(std::move(ident)),
    type(std::move(type)){}
  
  virtual ~ResolvedDecl() = default;

  virtual void dump(size_t level = 0) const = 0;
};

struct ResolvedNumberLiteral: public ResolvedExpr{
  double value;

  ResolvedNumberLiteral(int line, int col, double value)
  : ResolvedExpr(line, col, Type::builtinNumber()),
    value(value){}
  
  void dump(size_t level = 0) const override;
};

struct ResolvedDeclRefExpr: public ResolvedExpr{
  const ResolvedDecl *decl;

  ResolvedDeclRefExpr(int line, int col, ResolvedDecl &decl)
  : ResolvedExpr(line, col, decl.type),
    decl(&decl){}

  void dump(size_t level = 0) const override;
};

struct ResolvedBlock{
  int line;
  int col;
  std::vector<std::unique_ptr<ResolvedStmt>> statements;

  ResolvedBlock(int line, int col, std::vector<std::unique_ptr<ResolvedStmt>> statements)
  : line(line), col(col),
    statements(std::move(statements)){}

  void dump(size_t level = 0) const;
};

struct ResolvedParamaDecl: public ResolvedDecl{
  ResolvedParamaDecl(int line, int col, std::string ident, Type type)
  : ResolvedDecl{ line, col, std::move(ident), type }{}

  void dump(size_t level = 0) const override;
};

struct ResolvedFunctionDecl: public ResolvedDecl {
  std::vector<std::unique_ptr<ResolvedParamaDecl>> params;
  std::unique_ptr<ResolvedBlock> body;

  ResolvedFunctionDecl(int line, int col, 
  std::string ident,
  Type type,
  std::vector<std::unique_ptr<ResolvedParamaDecl>> params,
  std::unique_ptr<ResolvedBlock> body): 
  ResolvedDecl(line, col, std::move(ident), type),
  params(std::move(params)),
  body(std::move(body)){}

  void dump(size_t level = 0) const override;
};

struct ResolvedCallExpr: public ResolvedExpr{
  const ResolvedFunctionDecl *callee;
  std::vector<std::unique_ptr<ResolvedExpr>> arguments;

  ResolvedCallExpr(int line, int col, 
  const ResolvedFunctionDecl &callee,
  std::vector<std::unique_ptr<ResolvedExpr>> arguments)
  : ResolvedExpr(line, col, callee.type),
    callee(&callee),
    arguments(std::move(arguments)){}
  
  void dump(size_t level = 0) const override;
};

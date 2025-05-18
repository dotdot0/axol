#ifndef SEMA_H

#include <iostream>
#include <string>
#include <vector>
#include "lexer.h"
#include "type.h"
#include <memory>
#include <optional>
#include <llvm/Support/ErrorHandling.h>
#include "parser.h"

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

struct ResolvedGroupingExpr: public ResolvedExpr{
  std::unique_ptr<ResolvedExpr> expr;

  ResolvedGroupingExpr(int line, int col
  ,std::unique_ptr<ResolvedExpr> expr):
  ResolvedExpr(line,col, expr->type),
  expr(std::move(expr)){}

  void dump(std::size_t level = 0) const override;
};

struct ResolvedBinaryOperator: public ResolvedExpr{
  TokenKind op;
  std::unique_ptr<ResolvedExpr> lhs;
  std::unique_ptr<ResolvedExpr> rhs;

  ResolvedBinaryOperator(int line, int col, 
  TokenKind op,
  std::unique_ptr<ResolvedExpr> lhs,
  std::unique_ptr<ResolvedExpr> rhs)
  : ResolvedExpr(line, col, lhs->type),
    op(op),
    lhs(std::move(lhs)),
    rhs(std::move(rhs)){}
  
  void dump(std::size_t level = 0) const override;

};

struct ResolvedUnaryOperator: public ResolvedExpr{
  TokenKind op;
  std::unique_ptr<ResolvedExpr> operand;

  ResolvedUnaryOperator(int line, int col, TokenKind op,
  std::unique_ptr<ResolvedExpr> operand):
  ResolvedExpr(line, col, operand->type),
  op(op),
  operand(std::move(operand)){}

  void dump(std::size_t level = 0) const override;
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

struct ResolvedReturnStmt : public ResolvedStmt {
  std::unique_ptr<ResolvedExpr> expr;

  ResolvedReturnStmt(int line, int col, std::unique_ptr<ResolvedExpr> expr = nullptr)
  : ResolvedStmt(line,col),
    expr(std::move(expr)) {}
  
  void dump(size_t level = 0) const override;
};

class Sema {
  public:
  std::vector<std::unique_ptr<FunctionDecl>> ast;

  std::vector<std::vector<ResolvedDecl *>> scopes;

  ResolvedFunctionDecl *currentFunction;

  class ScopeRAII {
    Sema *sema;

    public:
      explicit ScopeRAII(Sema *sema)
      : sema(std::move(sema)) {
        sema->scopes.emplace_back();
      }
      ~ScopeRAII() { sema->scopes.pop_back(); }
  };

    explicit Sema(std::vector<std::unique_ptr<FunctionDecl>> ast)
    : ast(std::move(ast)){}

    std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolveAST();
    std::pair<ResolvedDecl *, int> lookupDecl(const std::string id);
    bool insertDeclToCurrentScope(ResolvedDecl &decl);
    std::unique_ptr<ResolvedFunctionDecl> createBuiltinPrintln();
    std::optional<Type> resolveType(Type parsedType);
    std::unique_ptr<ResolvedFunctionDecl> resolveFunctionDeclaration(const FunctionDecl &function);
    std::unique_ptr<ResolvedParamaDecl> resolveParamDecl(const ParamDecl &param);
    std::unique_ptr<ResolvedBlock> resolveBlock(const Block &block);
    std::unique_ptr<ResolvedStmt> resolveStmt(const Stmt &stmt);
    std::unique_ptr<ResolvedReturnStmt> resolveReturnStmt(const ReturnStmt &returnStmt);
    std::unique_ptr<ResolvedExpr> resolveExpr(const Expr &expr);
    std::unique_ptr<ResolvedDeclRefExpr> resolveDeclRefExpr(const DeclRefExpr &declRefExpr, bool isCallee = false);
    std::unique_ptr<ResolvedCallExpr> resolveCallExpr(const CallExpr &call);
    std::unique_ptr<ResolvedUnaryOperator> resolveUnaryOperator(const UnaryOperator &op);
    std::unique_ptr<ResolvedBinaryOperator> resolveBinaryOperator(const BinaryOperator &bin);
    std::unique_ptr<ResolvedGroupingExpr> resolveGroupingExpr(const GroupingExpr &g);
};

#endif
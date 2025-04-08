#include "../include/sema.h"
#include "../include/parser.h"


#define matchOrReturn(tok, msg) \
  if(nextToken.kind != tok) \
    report(nextToken.line, nextToken.col, msg);


#define varOrReturn(var, init) \
  auto var = (init); \
  if(!var) \
    return nullptr;
  

std::string_view getOpStrs(TokenKind op) {
  if (op == TokenKind::Plus)
    return "+";
  if (op == TokenKind::Minus)
    return "-";
  if (op == TokenKind::Asterisk)
    return "*";
  if (op == TokenKind::Slash)
    return "/";
}

std::string ident_s(std::size_t level) {
  return std::string(level * 2, ' ');
}

void ResolvedNumberLiteral::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedNumberLiteral: " << value << "\n";
}

void ResolvedBinaryOperator::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedBinaryOperator: '" << getOpStrs(op)
  << '\'' << '\n';

  lhs->dump(level + 1);
  rhs->dump(level + 1);
}

void ResolvedUnaryOperator::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedUnaryOperator: '" << getOpStrs(op)
  << '\'' << '\n';

  operand->dump(level + 1);
}

void ResolvedDeclRefExpr::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedDeclRefExpr: @(" << decl << ") "
    << decl->ident << '\n';
}

void ResolvedCallExpr::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedCallExpr: @" << callee << ") "
  << callee->ident << '\n';

  for (auto &&arg: arguments)
    arg->dump(level+1);
}

void ResolvedBlock::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedBlock\n";

  for(auto &&stmt: statements)
    stmt->dump(level+1);
}

void ResolvedParamaDecl::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedParamDecl: @(" << this << ")"
  << ident << ":" << type.name << '\n'; 
}

void ResolvedFunctionDecl::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedFunctionDecl: @(" << this << ") "
  << ident << ":" << type.name << '\n';

  for(auto &param: params){
    param->dump(level+1);
  }

  if(body)
    body->dump(level+1);
}

void ResolvedReturnStmt::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedReturnStmt\n";

  if(expr)
    expr->dump(level+1);
}

std::pair<ResolvedDecl *, int> Sema::lookupDecl(const std::string id) {
  int scopeIdx = 0;
  for(auto it = scopes.rbegin(); it != scopes.rend(); ++it){
    for(auto &&decl: *it){
      if(decl->ident != id)
        continue;
      return {decl, scopeIdx};
    }
    ++scopeIdx;
  }
  return {nullptr, -1};
}

bool Sema::insertDeclToCurrentScope(ResolvedDecl &decl) {
  const auto &[foundDecl, scopeIdx] = lookupDecl(decl.ident);

  if(foundDecl && scopeIdx == 0){
    report(decl.line, decl.col, "redeclaration of '" + decl.ident + '\'');
    return false;
  }
  // if(scopes.empty()){
  //   scopes.emplace_back();
  // }
  scopes.back().emplace_back(&decl);
  return true;
}

std::unique_ptr<ResolvedFunctionDecl> Sema::createBuiltinPrintln() {
  int line, col = 0;
  auto param = std::make_unique<ResolvedParamaDecl>(line, col, "n", Type::builtinNumber());
  std::vector<std::unique_ptr<ResolvedParamaDecl>> params;
  params.emplace_back(std::move(param));

  auto block = std::make_unique<ResolvedBlock>(
    line, col, std::vector<std::unique_ptr<ResolvedStmt>>());
  
  return std::make_unique<ResolvedFunctionDecl>(
    line, col, "println", Type::builtinVoid(), std::move(params), std::move(block)
  );
}

std::optional<Type> Sema::resolveType(Type parsedType) {
  // std::cout << parsedType.name << "\n";
  if (parsedType.kind == Type::Kind::Number || parsedType.name == "number") {
    return Type::builtinNumber(); // Built-in type 'number'
  }
  if(parsedType.kind == Type::Kind::Custom)
   return std::nullopt;
  return parsedType;
}

std::unique_ptr<ResolvedCallExpr> Sema::resolveCallExpr(const CallExpr &call) {
  const auto *dre = dynamic_cast<const DeclRefExpr *>(call.callee.get());

  std::cout << dre->identifier << "\n";

  if(!dre)
    return report(call.line, call.col, "expression cannot be called as a function.");
  
  varOrReturn(resolvedCallee, resolveDeclRefExpr(*dre, true));

  const auto *resolvedFunctionDecl = dynamic_cast<const ResolvedFunctionDecl *>(resolvedCallee->decl);

  std::cout << resolvedFunctionDecl->ident << "\n";

  if(!resolvedFunctionDecl)
    return report(call.line, call.col, "calling non-function type");
  
  if(call.arguments.size() != resolvedFunctionDecl->params.size())
    return report(call.line, call.col, "argument count mismatch in function call");
  
  std::vector<std::unique_ptr<ResolvedExpr>> resolvedArguments;
  int idx = 0;
  for(auto &&arg: call.arguments){
    varOrReturn(resolvedArg, resolveExpr(*arg));

    if(resolvedArg->type.kind != resolvedFunctionDecl->params[idx]->type.kind)
      return report(resolvedArg->line, resolvedArg->col, "unexpected type of argument.");
    
    ++idx;
    resolvedArguments.emplace_back(std::move(resolvedArg));
  }
  return std::make_unique<ResolvedCallExpr>(
    call.line, call.col, *resolvedFunctionDecl, std::move(resolvedArguments)
  );
}

std::unique_ptr<ResolvedDeclRefExpr> Sema::resolveDeclRefExpr(const DeclRefExpr &declRefExpr, bool isCallee) {
  ResolvedDecl *decl = lookupDecl(declRefExpr.identifier).first;
  if(!decl)
    return report(declRefExpr.line, declRefExpr.col, "symbol '"
    + declRefExpr.identifier + "' not found.");
  
  if(!isCallee && dynamic_cast<ResolvedFunctionDecl *>(decl))
    return report(declRefExpr.line, declRefExpr.col, "expected to call function '"
    + declRefExpr.identifier + "'");
  
  return std::make_unique<ResolvedDeclRefExpr>(declRefExpr.line, declRefExpr.col, *decl);
}

std::unique_ptr<ResolvedExpr> Sema::resolveExpr(const Expr &expr) {
  if(const auto *number = dynamic_cast<const NumberLiteral *>(&expr))
    return std::make_unique<ResolvedNumberLiteral>(number->line, number->col, std::stod(number->value));
  
  if (const auto *declRefExpr = dynamic_cast<const DeclRefExpr *>(&expr))
    return resolveDeclRefExpr(*declRefExpr);
  
  if(const auto *callRefExpr = dynamic_cast<const CallExpr *>(&expr))
    return resolveCallExpr(*callRefExpr);
  
  if(const auto *op = dynamic_cast<const UnaryOperator *>(&expr))
    return resolveUnaryOperator(*op);
  
  if(const auto *bin = dynamic_cast<const BinaryOperator *>(&expr))
    return resolveBinaryOperator(*bin);

  llvm_unreachable("unexpected expression");
}

std::unique_ptr<ResolvedUnaryOperator> Sema::resolveUnaryOperator(const UnaryOperator &op){
  varOrReturn(resolvedRHS, resolveExpr(*op.operand));

  if(resolvedRHS->type.kind == Type::Kind::Void)
    return report(resolvedRHS->line, resolvedRHS->col,
    "void expression cannot be used as an operand to unary operator");
  
  return std::make_unique<ResolvedUnaryOperator>(op.line, op.col, op.op,std::move(resolvedRHS));
}

std::unique_ptr<ResolvedBinaryOperator>
Sema::resolveBinaryOperator(const BinaryOperator &binop) {
  varOrReturn(resolvedLHS, resolveExpr(*binop.lhs));
  varOrReturn(resolvedRHS, resolveExpr(*binop.rhs));

  if (resolvedLHS->type.kind == Type::Kind::Void)
    return report(resolvedLHS->line, resolvedRHS->col,
    "void expression cannot be used as LHS operand to binary operator");

  if (resolvedRHS->type.kind == Type::Kind::Void)
    return report(resolvedRHS->line, resolvedRHS->col,
    "void expression cannot be used as RHS operand to binary operator");

  return std::make_unique<ResolvedBinaryOperator>(binop.line, binop.col, binop.op,std::move(resolvedLHS), std::move(resolvedRHS));
}

std::unique_ptr<ResolvedReturnStmt> Sema::resolveReturnStmt(const ReturnStmt &returnStmt) {
  std::unique_ptr<ResolvedExpr> resolvedExpr;
  if(returnStmt.expr) {
    resolvedExpr = resolveExpr(*returnStmt.expr);
    if(!resolvedExpr)
      return nullptr;
    
    if(currentFunction->type.kind != resolvedExpr->type.kind)
      return report(resolvedExpr->line, resolvedExpr->col, "Unexpected Token Type");
  }

  return std::make_unique<ResolvedReturnStmt>(returnStmt.line, returnStmt.col, std::move(resolvedExpr));
}

std::unique_ptr<ResolvedParamaDecl> Sema::resolveParamDecl(const ParamDecl &param) {
  std::optional<Type> type = resolveType(param.type);

  if(!type || param.type.kind == Type::Kind::Void)
    return report(param.line, param.col, "parameter '" + param.ident +
    "' has invalid '" + param.type.name + "' type");
  
  return std::make_unique<ResolvedParamaDecl>(param.line, param.col, param.ident, *type);
}

std::unique_ptr<ResolvedStmt> Sema::resolveStmt(const Stmt &stmt) {
  if(auto *expr = dynamic_cast<const Expr *>(&stmt))
    return resolveExpr(*expr);
  
  if(auto *returnStmt = dynamic_cast<const ReturnStmt *>(&stmt))
    return resolveReturnStmt(*returnStmt);

  llvm_unreachable("unexpected statement");
}

std::unique_ptr<ResolvedBlock> Sema::resolveBlock(const Block &block) {
  std::vector<std::unique_ptr<ResolvedStmt>> resolvedStmts;

  bool error = false;

  int reportUnreachableCount = 0;

  ScopeRAII blockScope(this);
  for(auto &&stmt: block.statements) {
    auto resolvedStmt = resolveStmt(*stmt);

    error |= !resolvedStmts.emplace_back(std::move(resolvedStmt));
    if(error)
      continue;
    
    if(reportUnreachableCount == 1){
      report(stmt->line, stmt->col, "Unreachable statement", true);
      ++reportUnreachableCount;
    }
    if(dynamic_cast<ReturnStmt *>(stmt.get()))
      ++reportUnreachableCount;
  }
  if(error)
    return nullptr;

  return std::make_unique<ResolvedBlock>(block.line, block.col, std::move(resolvedStmts));
}

std::unique_ptr<ResolvedFunctionDecl> Sema::resolveFunctionDeclaration(const FunctionDecl &function) {
  for(auto it: scopes){
    for(auto i: it){
      std::cout << i->ident << "\n";
    }
  }
  std::optional<Type> type = resolveType(function.type);
  // std::cout << type.value().name << "\n";

  if(!type)
    return report(function.line, function.col, "function '" + function.ident 
    + "' has invalid '" + function.type.name + "' type");
  
  if(function.ident == "main") {
    if(type->kind != Type::Kind::Void)
      return report(function.line, function.col, "'main' function is expected to have 'void' type");
    
    if(!function.params.empty())
      return report(function.line, function.col, "'main' function is expected to have no arguments");
  }

  std::vector<std::unique_ptr<ResolvedParamaDecl>> resolvedParams;

  ScopeRAII paramScope(this);

  for(auto &&param: function.params){
    auto resolvedParam = resolveParamDecl(*param);

    if(!resolvedParam || !insertDeclToCurrentScope(*resolvedParam))
      return nullptr;
    
    resolvedParams.emplace_back(std::move(resolvedParam));
  }

  return std::make_unique<ResolvedFunctionDecl>(
    function.line, function.col, function.ident, *type, std::move(resolvedParams),
    nullptr
  );
}

std::vector<std::unique_ptr<ResolvedFunctionDecl>> Sema::resolveAST() {
  std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolvedTree;
  auto println = createBuiltinPrintln();

  ScopeRAII gloabalScope(this);
  insertDeclToCurrentScope(*resolvedTree.emplace_back(std::move(println)));

  bool error = false;

  for(auto &&fn: ast) {
    auto resolvedFunctionDecl = resolveFunctionDeclaration(*fn);
    if(!resolvedFunctionDecl || !insertDeclToCurrentScope(*resolvedFunctionDecl)){
      error = true;
      continue;
    }
    resolvedTree.emplace_back(std::move(resolvedFunctionDecl));
  }

  if (error)
    return {};

  for(size_t i = 1; i < resolvedTree.size(); ++i){
    currentFunction = resolvedTree[i].get();

    ScopeRAII(this);
    for(auto &&param : currentFunction->params){
      insertDeclToCurrentScope(*param);
    }

    auto resolvedBody = resolveBlock(*ast[i - 1]->body);
    if(!resolvedBody){
      error = true;
      continue;
    }
    currentFunction->body = std::move(resolvedBody);
  }

  if(error)
    return {};

  return resolvedTree;
}
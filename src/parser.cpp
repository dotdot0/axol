#include "parser.h"

#define matchOrReturn(tok, msg) \
  if(nextToken.kind != tok) \
    report(nextToken.line, nextToken.col, msg);


#define varOrReturn(var, init) \
  auto var = (init); \
  if(!var) \
    return nullptr;


std::string ident_(std::size_t level) {
  return std::string(level * 2, ' ');
}

void FunctionDecl::dump(std::size_t level) const {
  std::cerr << ident_(level) << "FunctionDecl: " << ident << ":" << type.name << '\n';
  std::cerr << ident_(level+1) << "Params:" << "\n";

  for(auto &&param: params)
    param->dump(level+2);

  body->dump(level+1);
}

void ReturnStmt::dump(std::size_t level) const {
  std::cerr << ident_(level) << "ReturnStmt\n";
  if(expr)
    expr->dump(level+1);
}

void Block::dump(std::size_t level) const {
  std::cerr << ident_(level) << "Block\n";
  for(auto &&stmt: statements)
      stmt->dump(level+1); 

}

void NumberLiteral::dump(std::size_t level) const {
  std::cerr << ident_(level) << "NumberLiteral: " << value << "\n";
}

void DeclRefExpr::dump(std::size_t level) const {
  std::cerr << ident_(level) << "DeclRefExpr: " << identifier << "\n";
}

void CallExpr::dump(std::size_t level) const {
  std::cerr << ident_(level) << "CallExpr:\n";

  callee->dump(level+1);

  std::cerr << ident_(level+1) << "ArgumentList:" << "\n";
  for(auto &&arg: arguments)
    arg->dump(level+2);
}

void ParamDecl::dump(std::size_t level) const {
  std::cerr << ident_(level) << "ParamDecl: " << "\n";
  std::cerr << ident_(level+1) << ident << ": " << type.name << "\n";
}

void ResolvedNumberLiteral::dump(std::size_t level) const {
  std::cerr << ident_(level) << "ResolvedNumberLiteral: " << value << "\n";
}

void ResolvedDeclRefExpr::dump(std::size_t level) const {
  std::cerr << ident_(level) << "ResolvedDeclRedExpr: @(" << decl << ") " << decl->ident << "\n";
}

void ResolvedBlock::dump(std::size_t level) const {
  std::cerr << ident_(level) << "ResolvedBlock\n";

  for(auto &&stmt: statements)
    stmt->dump(level+1);
  
}

void ResolvedParamDecl::dump(std::size_t level) const {
  std::cerr << ident_(level) << "ResolvedParamDecl: @(" << this << ") " << ident << ":" << "\n";
}

void ResolvedFunctionDecl::dump(std::size_t level) const {
  std::cerr << ident_(level) << "ResolvedFunctionDecl: " << ident << ":" << "\n";

  for(auto &&param: params)
    param->dump(level+1);
  
  body->dump(level+1);
}

void ResolvedReturnStmt::dump(std::size_t level) const {
  std::cerr << ident_(level) << "ResolvedReturnStmt\n";

  if(expr)
    expr->dump(level+1);
}

void Parser::eatNextToken(){
  nextToken = lexer->getNextToken();
}

std::nullptr_t report(int line, int col, std::string_view message, bool isWarning = false){
  std::cerr << line << ":" << col << ": " << (isWarning ? "warning: " : "error: ")
  << message << "\n";
  return nullptr;
}

void Parser::synchronizeOn(TokenKind kind){
  isIncompAST = true;
  while(nextToken.kind != TokenKind::Func && nextToken.kind != TokenKind::Eof) eatNextToken();
}

void Parser::synchronize(){
  isIncompAST = true;
  int braces = 0;
  while(true){
    TokenKind kind = nextToken.kind;

    if(kind == TokenKind::Lbrace) ++braces;

    else if(kind == TokenKind::Rbrace) {
      if(braces == 0) break;
      if(braces == 1) {
        eatNextToken();
        break;
      }else if(kind == TokenKind::SemiColon && braces == 0){
        eatNextToken();
        break;
      }else if(kind == TokenKind::Func || kind == TokenKind::Eof)
        break;
      --braces;
    }
  }
}

std::unique_ptr<Expr> Parser::parsePrimary(){
  int line = nextToken.line;
  int col  = nextToken.col;

  if(nextToken.kind == TokenKind::Number){
    auto literal = std::make_unique<NumberLiteral>(line, col, nextToken.value.value());
    eatNextToken();
    return literal;
  }

  if(nextToken.kind == TokenKind::Ident){
    auto literal = std::make_unique<DeclRefExpr>(line, col, *nextToken.value);
    eatNextToken();
    return literal;
  }

  return report(nextToken.line, nextToken.col, "expected expr");
}

std::unique_ptr<std::vector<std::unique_ptr<Expr>>> 
Parser::parseArgumentList(){
  matchOrReturn(TokenKind::Lpar, "expected '('");
  eatNextToken();

  std::vector<std::unique_ptr<Expr>> argumentList;

  while(true){
    if(nextToken.kind == TokenKind::Rpar)
      break;

    varOrReturn(expr, parseExpr());
    argumentList.emplace_back(std::move(expr));

    if(nextToken.kind != TokenKind::Comma)
      break;
    
    eatNextToken();
  }

  matchOrReturn(TokenKind::Rpar, "expected ')'");
  eatNextToken();

  return std::make_unique<std::vector<std::unique_ptr<Expr>>>(std::move(argumentList));
}

std::unique_ptr<Expr> Parser::parsePostfixExpr(){
  varOrReturn(expr, parsePrimary());

  if (nextToken.kind != TokenKind::Lpar)
    return expr;

  int line = nextToken.line;
  int col  = nextToken.col;
  varOrReturn(argumentList, parseArgumentList());

  return std::make_unique<CallExpr>(line, col, std::move(expr), std::move(*argumentList));
}

std::unique_ptr<ParamDecl> Parser::parseParamDecl(){
  int line = nextToken.line;
  int col  = nextToken.col;

  std::string ident = *nextToken.value;
  eatNextToken();

  matchOrReturn(TokenKind::Colon, "expected ':'");
  eatNextToken();

  varOrReturn(type, parseType());

  return std::make_unique<ParamDecl>(line, col, std::move(ident), std::move(*type));
}

std::optional<Type> Parser::parseType(){
  TokenKind kind = nextToken.kind;

  if(kind == TokenKind::Number){
    eatNextToken();
    return Type::builtinNumber();
  }

  if(kind == TokenKind::Void){
    eatNextToken();
    return Type::builtinVoid();
  }

  if(kind == TokenKind::Ident){
    auto t = Type::custom(*nextToken.value);
    eatNextToken();
    return t;
  }

  report(nextToken.line, nextToken.col, "expected type specifier");
  return std::nullopt;
}

std::unique_ptr<Expr> Parser::parseExpr(){
  return Parser::parsePostfixExpr();
}

std::unique_ptr<ReturnStmt> Parser::parseReturnStmt(){
  int line = nextToken.line;
  int col = nextToken.col;
  eatNextToken();

  std::unique_ptr<Expr> expr;
  if(nextToken.kind != TokenKind::SemiColon){
    expr = parseExpr();
    if(!expr)
      return nullptr;
  }

  matchOrReturn(TokenKind::SemiColon, "expected ';' at the end of a return statement")
  eatNextToken();

  return std::make_unique<ReturnStmt>(line, col, std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseStmt(){
  if(nextToken.kind == TokenKind::Return){
    return parseReturnStmt();
  }
  varOrReturn(expr, parseExpr());
  matchOrReturn(TokenKind::SemiColon, "expected ';' at the end of expr");
  eatNextToken();

  return expr;
}

std::unique_ptr<Block> Parser::parseBlock(){
  int line = nextToken.line;
  int col = nextToken.col;
  std::vector<std::unique_ptr<Stmt>> statements;
  eatNextToken();
  while(true){
    if(nextToken.kind == TokenKind::Rbrace) break;

    if(nextToken.kind == TokenKind::Eof || nextToken.kind == TokenKind::Func)
      return report(nextToken.line, nextToken.col, "expected '}' at the end of the block.");
    
    varOrReturn(stmt, parseStmt());
    if(!stmt) {
      synchronize();
    }
    statements.emplace_back(std::move(stmt)); 
  }
  eatNextToken();

  return std::make_unique<Block>(line, col, std::move(statements));
}

std::unique_ptr<std::vector<std::unique_ptr<ParamDecl>>>
Parser::parseParamList(){
  matchOrReturn(TokenKind::Lpar, "expected '('");
  eatNextToken();

  std::vector<std::unique_ptr<ParamDecl>> paramList;

  while(true){
    if(nextToken.kind == TokenKind::Rpar)
      break;
    
    matchOrReturn(TokenKind::Ident, "expected param decl");

    varOrReturn(paramDecl, parseParamDecl());
    paramList.emplace_back(std::move(paramDecl));

    if(nextToken.kind != TokenKind::Comma) 
      break;
    
    eatNextToken();
  }

  matchOrReturn(TokenKind::Rpar, "expected ')'");
  eatNextToken();

  return std::make_unique<std::vector<std::unique_ptr<ParamDecl>>>(std::move(paramList));
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl(){
  int line = nextToken.line;
  int col = nextToken.col;
  eatNextToken();
  matchOrReturn(TokenKind::Ident, "expected ident");
  std::string functionIdent = *nextToken.value;
  eatNextToken();

  varOrReturn(paramList, parseParamList());

  matchOrReturn(TokenKind::Colon, "expected ':'");
  eatNextToken();

  varOrReturn(type, parseType());

  matchOrReturn(TokenKind::Lbrace, "expected function body");
  varOrReturn(block, parseBlock());

  return std::make_unique<FunctionDecl>(line, col, functionIdent, *type, std::move(*paramList), std::move(block));
}

std::pair<std::vector<std::unique_ptr<FunctionDecl>>, bool>
Parser::parseSourceFile(){
  bool hasMainFunction = false;
  std::vector<std::unique_ptr<FunctionDecl>> functions;
  while(nextToken.kind!=TokenKind::Eof){
    if(nextToken.kind != TokenKind::Func){
      report(nextToken.line, nextToken.col, "only function decl are allowed on the top level"); 
      synchronizeOn(TokenKind::Func);
      continue;
    }

    auto fn = parseFunctionDecl();
    if(!fn){
      synchronizeOn(TokenKind::Func);
      continue;
    }


    functions.emplace_back(std::move(fn));

    for(auto &fn: functions){
      hasMainFunction |= fn->ident=="main";
    }

    if(!hasMainFunction && !isIncompAST) report(nextToken.line, nextToken.col, "main func not found");

  }

  return {std::move(functions), !isIncompAST && hasMainFunction};
}
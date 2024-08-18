#include "parser.h"

#define matchOrReturn(tok, msg)                                                \
  if (nextToken.kind != tok)                                                   \
    return report(nextToken.line, nextToken.col, msg);


#define varOrReturn(var, init) \
  auto var = (init); \
  if(!var) \
    return nullptr;


std::string ident_(std::size_t level) {
  return std::string(level * 2, ' ');
}

void FunctionDecl::dump(std::size_t level) const {
  std::cerr << ident_(level) << "FunctionDecl: " << ident << ":" << type.name << '\n';
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


std::unique_ptr<Expr> Parser::parsePrimary(){
  int line = nextToken.line;
  int col  = nextToken.col;

  if(nextToken.kind == TokenKind::Number){
    auto literal = std::make_unique<NumberLiteral>(line, col, *nextToken.value);
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

std::optional<Type> Parser::parseType(){
  TokenKind kind = nextToken.kind;

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
  return Parser::parsePrimary();
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
    statements.emplace_back(std::move(stmt)); 
  }
  eatNextToken();

  return std::make_unique<Block>(line, col, std::move(statements));
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl(){
  int line = nextToken.line;
  int col = nextToken.col;
  eatNextToken();
  matchOrReturn(TokenKind::Ident, "expected ident");
  std::string functionIdent = *nextToken.value;
  eatNextToken();
  matchOrReturn(TokenKind::Lpar, "expected '('");
  eatNextToken();

  matchOrReturn(TokenKind::Rpar, "expected ')'");
  eatNextToken();

  matchOrReturn(TokenKind::Colon, "expected ':'");
  eatNextToken();

  varOrReturn(type, parseType());

  matchOrReturn(TokenKind::Lbrace, "expected function body");
  varOrReturn(block, parseBlock());

  return std::make_unique<FunctionDecl>(line, col, functionIdent, *type, std::move(block));
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
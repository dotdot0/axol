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

void Block::dump(std::size_t level) const{
  std::cerr << ident_(level) << "Block\n";
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

std::unique_ptr<Block> Parser::parseBlock(){
  int line = nextToken.line;
  int col = nextToken.col;
  eatNextToken();
  matchOrReturn(TokenKind::Rbrace, "expected '}' at the end of a block");
  eatNextToken();

  return std::make_unique<Block>(line, col);
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
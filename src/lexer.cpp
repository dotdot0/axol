#include "lexer.h"

Token Lexer::getNextToken(){
  char curr = consume();
  while(isSpace(curr)) curr = consume();

  if(isAlpha(curr)){
    std::string value{curr};
    while(isAlNum(peekNextChar())){
      curr = consume();
      value += curr;
    }
    if(keywords.count(value)) 
      return Token{line, col, keywords.at(value), std::move(value)};

    return Token{line, col, TokenKind::Ident, std::move(value)};
  }

  if(isNum(curr)){
    std::string value{curr};

    while(isNum(peekNextChar()))
      value+=consume();
    
    if(peekNextChar() != '.') 
      return Token{line, col, TokenKind::Number, std::move(value)};
    
    value+=consume();
    if(!isNum(peekNextChar()))
      return Token{line, col, TokenKind::Unk};

    while(isNum(peekNextChar()))
      value+=consume();
    
    return Token{line, col, TokenKind::Number, std::move(value)};
  }

  for(auto &c: charTokens) 
    if(c == curr) return Token{ line, col, static_cast<TokenKind>(c) };
  
  if(curr == '#'){
    while(peekNextChar() != '\n' && peekNextChar() != '\0') consume();
    return getNextToken();
  }

  return Token{line, col, TokenKind::Unk};

}
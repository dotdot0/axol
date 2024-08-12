#include "lexer.h"

Token Lexer::getNextToken(){
  char curr = consume();

  while(isSpace(curr)) curr = consume();

  if(isAlpha(curr)){
    std::string value{curr};
    while(isAlNum(curr))
      value+=consume();

    if(keywords.count(value)) return Token{line, col, keywords.at(value), std::move(value)};
    return Token{line, col, keywords.at(value), std::move(value)};
  }

  for(auto &c: charTokens) 
    if(c == curr) return Token{ line, col, static_cast<TokenKind>(c) };
  
  if(curr == '#'){
    while(peekNextChar() != '\n' && peekNextChar() != '\0') consume();
    return getNextToken();
  }


}
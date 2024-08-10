#ifndef LEXER_H

#include <string>
#include <unordered_map>

constexpr char charTokens[] = {'\0', '(', ')', '{', '}', ':'};

enum class TokenKind: char{
  Ident,
  Func,
  Void,
  Eof    = charTokens[0],
  Lpar   = charTokens[1],
  Rpar   = charTokens[2], 
  Lbrace = charTokens[3],
  Rbrace = charTokens[4],
  Colon  = charTokens[5],
  Unk    = -128
};

const std::unordered_map<std::string, TokenKind> keywords = {
  {"void", TokenKind::Void},
  {"func", TokenKind::Func}
};


#endif
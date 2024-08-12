#ifndef LEXER_H

#include <string>
#include <unordered_map>
#include <optional>
#include <cassert>

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

struct SourceFile{
  std::string_view path;
  std::string buffer;
};

struct Token{
  int line;
  int col;
  TokenKind kind;
  std::optional<std::string> value = std::nullopt;
};

class Lexer{
  const SourceFile *source;
  std::size_t idx = 0; 
  int line = 1;
  int col = 0;
  public:
    Lexer(const SourceFile &source) : source(&source) { } 

    Token getNextToken();

    char peekNextChar() const { if(idx > source->buffer.length()) return -1; else return source->buffer[idx];  } 

    char consume(){
      ++col;

      if(source->buffer[idx] == '\n') {
        ++line;
        col = 0;
      }

      return source->buffer[idx++];
  }

  bool isSpace(char c) { return c==' ' || c=='\f' || c=='\n' || c=='\r' || c=='\v'; }

  bool isAlpha(char c) {
    return ( ('a' <= c && c <= 'z') || ('A' <= c <= 'Z') );
  }

  bool isNum(char c) { return '0' <= c && c <= '9'; }
  bool isAlNum(char c) { return isAlpha(c) || isNum(c); }
};

const std::unordered_map<std::string, TokenKind> keywords = {
  {"void", TokenKind::Void},
  {"func", TokenKind::Func}
};


#endif
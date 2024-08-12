#ifndef LEXER_H

#include <string>
#include <unordered_map>
#include <optional>
#include <iostream>

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

struct SourceFile{
  std::string_view path;
  std::string buffer;
  SourceFile(const std::string_view path, const std::string buffer): path(path), buffer(buffer) {};
};

struct Token{
  int line;
  int col;
  TokenKind kind;
  std::optional<std::string> value = std::nullopt;
  std::string to_string(){
    switch (kind)
    {
    case TokenKind::Func:
      return "Func";
    case TokenKind::Void:
      return "Void";
    case TokenKind::Ident:
      return "Ident";
    default:
      return "single";
    }
  }
};

class Lexer{
  public:
  const SourceFile *source;
  std::size_t idx = 0; 
  int line = 1;
  int col = 0;
    explicit Lexer(const std::string_view &path, const std::string &buffer) {
      this->source = new SourceFile(path, buffer);
    };

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
    return ( ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') );
  }

  bool isNum(char c) { return '0' <= c && c <= '9'; }
  bool isAlNum(char c) { return isAlpha(c) || isNum(c); }
};




#endif
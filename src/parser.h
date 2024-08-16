#ifndef PARSER_H

#include<iostream>
#include<string>
#include<memory>
#include"lexer.h"
#include<vector>
#include"type.h"


struct Decl{
  int line;
  int col;
  std::string ident;

  Decl(int line, int col, std::string ident): ident(std::move(ident)) {
    this->line = line;
    this->col = col;
  }

  virtual ~Decl() = default;

  virtual void dump(std::size_t level = 0) const = 0;
};

struct Block{
  int line;
  int col;
  Block(int line, int col) {
    this->line = line;
    this->col = col;
  }
  void dump (std::size_t level) const;
};

struct FunctionDecl: public Decl{
  Type type;
  std::unique_ptr<Block> body;

  FunctionDecl(int line, int col, std::string ident, Type type, std::unique_ptr<Block> body): Decl(line, col, std::move(ident))
  , body(std::move(body)), type(std::move(type)){}

  void dump(std::size_t level = 0) const override;
};

class Parser{
  Lexer *lexer;
  Token nextToken;
  bool isIncompAST = false;
  public:
  explicit Parser(Lexer &lexer): lexer(&lexer), nextToken(lexer.getNextToken()){}
  void eatNextToken();
  std::pair<std::vector<std::unique_ptr<FunctionDecl>>, bool> 
  parseSourceFile();
  void synchronizeOn(TokenKind kind);
  std::unique_ptr<FunctionDecl> parseFunctionDecl();
  std::optional<Type> parseType();
  std::unique_ptr<Block> parseBlock();
};

#endif
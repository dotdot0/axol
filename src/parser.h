#ifndef PARSER_H

#include<iostream>
#include<string>
#include<memory>

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
  std::unique_ptr<Block> body;

  FunctionDecl(int line, int col, std::string ident, std::unique_ptr<Block> body): Decl(line, col, std::move(ident))
  , body(std::move(body)){}

  void dump(std::size_t level = 0) const override;
};

#endif
#include "parser.h"

std::string ident_(std::size_t level) {
  return std::string(level * 2, ' ');
}

void FunctionDecl::dump(std::size_t level) const {
  std::cerr << ident_(level) << "FunctionDecl: " <<   ident << ":" << '\n';
  body->dump(level+1);
}

void Block::dump(std::size_t level) const{
  std::cerr << ident_(level) << "Block\n";
}
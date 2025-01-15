#include "../include/sema.h"

std::string ident_s(std::size_t level) {
  return std::string(level * 2, ' ');
}

void ResolvedNumberLiteral::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedNumberLiteral: " << value << "\n";
}

void ResolvedDeclRefExpr::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedDeclRefExpr: @(" << decl << ") "
    << decl->ident << '\n';
}

void ResolvedCallExpr::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedCallExpr: @" << callee << ") "
  << callee->ident << '\n';

  for (auto &&arg: arguments)
    arg->dump(level+1);
}

void ResolvedBlock::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedBlock\n";

  for(auto &&stmt: statements)
    stmt->dump(level+1);
}

void ResolvedParamaDecl::dump(size_t level) const {
  std::cerr << ident_s(level) << "ResolvedParamDecl: @(" << this << ")"
  << ident << ":" << type.name << '\n'; 
}
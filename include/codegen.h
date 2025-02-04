#ifndef CODEGEN_H
#define CODEGEN_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/TargetParser/Host.h"
#include "sema.h"

class Codegen{
  std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolvedAST;

  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  llvm::Module module;

  public:
    Codegen(std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolvedAST, std::string_view source_path);
    llvm::Module *generateIR();
};

#endif
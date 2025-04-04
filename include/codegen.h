#ifndef CODEGEN_H
#define CODEGEN_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/TargetParser/Host.h"
#include "sema.h"
#include <map>

class Codegen{
  std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolvedAST;
  std::map<const ResolvedDecl *, llvm::Value *> declarations;

  llvm::Instruction *allocaInsertPoint;

  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  llvm::Module module;

  llvm::Value *retVal = nullptr;
  llvm::BasicBlock *retBB = nullptr;

  public:
    Codegen(std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolvedAST, std::string_view source_path);
    llvm::Module *generateIR();
    llvm::Type *generateType(Type type);
    void generateFunctionDecl(const ResolvedFunctionDecl &functionDecl);
    void generateFunctionBody(const ResolvedFunctionDecl &functionDecl);
    llvm::AllocaInst * allocateStackVariable(llvm::Function *function, const std::string_view ident);
    void generateBlock(const ResolvedBlock &block);
    llvm::Value *generateStmt(const ResolvedStmt &stmt);
    llvm::Value *generateReturnStmt(const ResolvedReturnStmt &returnStmt);
    llvm::Value *generateExpr(const ResolvedExpr &expr);
    llvm::Value *generateCallExpr(const ResolvedCallExpr &expr);
    void generateBuiltinPrintBody(const ResolvedFunctionDecl &println);
    void generateMainWrapper();
};

#endif
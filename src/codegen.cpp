#include "../include/codegen.h"

Codegen::Codegen(std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolvedAST, std::string_view sourcePath):
resolvedAST(std::move(resolvedAST)),
builder(context),
module("<translation_unit>", context){
  module.setSourceFileName(sourcePath);
  module.setTargetTriple(llvm::sys::getDefaultTargetTriple());
}

llvm::Type *Codegen::generateType(Type type){
  if(type.kind == Type::Kind::Number)
    return builder.getDoubleTy();
  
  return builder.getVoidTy();
}

void Codegen::generateFunctionDecl(const ResolvedFunctionDecl &functionDecl) {
  auto *retType = generateType(functionDecl.type);
  for(auto &&param: functionDecl.params){}
}

llvm::Module *Codegen::generateIR() {
  for(auto &&function: resolvedAST){
    
  }

  for(auto &&function: resolvedAST){

  }
  
  return &module;
}
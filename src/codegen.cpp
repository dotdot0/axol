#include "../include/codegen.h"

Codegen::Codegen(std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolvedAST, std::string_view sourcePath):
resolvedAST(std::move(resolvedAST)),
builder(context),
module("<translation_unit>", context){
  module.setSourceFileName(sourcePath);
  module.setTargetTriple(llvm::sys::getDefaultTargetTriple());
}
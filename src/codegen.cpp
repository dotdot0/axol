#include "../include/codegen.h"

llvm::Value *Codegen::doubleToBool(llvm::Value *v) {
  return builder.CreateFCmpONE(v,
  llvm::ConstantFP::get(builder.getDoubleTy(), 0.0), "to.bool");
}

llvm::Value *Codegen::boolToDouble(llvm::Value *v) {
  return builder.CreateUIToFP(v, builder.getDoubleTy(), "to.double");
}

Codegen::Codegen(std::vector<std::unique_ptr<ResolvedFunctionDecl>> resolvedAST, std::string_view sourcePath):
resolvedAST(std::move(resolvedAST)),
builder(context),
module("<translation_unit>", context){
  module.setSourceFileName(sourcePath);
  module.setTargetTriple(llvm::sys::getDefaultTargetTriple());
}

void Codegen::generateMainWrapper(){
  auto *builtinMain = module.getFunction("main");
  builtinMain->setName("__builtin_main");

  auto *main = llvm::Function::Create(llvm::FunctionType::get(builder.getInt32Ty(), {}, false),
  llvm::Function::ExternalLinkage, "main", module);

  auto *entry = llvm::BasicBlock::Create(context, "entry", main);
  builder.SetInsertPoint(entry);

  builder.CreateCall(builtinMain);
  builder.CreateRet(llvm::ConstantInt::getSigned(builder.getInt32Ty(), 0));
}

void Codegen::generateBuiltinPrintBody(const ResolvedFunctionDecl &println){
  auto *type = llvm::FunctionType::get(builder.getInt32Ty(), {llvm::PointerType::get(builder.getInt8Ty(), 0)}, true);
  auto *printf = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "printf", module);
  auto *format = builder.CreateGlobalStringPtr("%.15g\n");
  llvm::Value *param = builder.CreateLoad(builder.getDoubleTy(), declarations[println.params[0].get()]);
  builder.CreateCall(printf, {format, param});
}

llvm::AllocaInst * Codegen::allocateStackVariable(llvm::Function *function, const std::string_view ident){
  llvm::IRBuilder<> tmpBuilder(context);
  tmpBuilder.SetInsertPoint(allocaInsertPoint);

  return tmpBuilder.CreateAlloca(tmpBuilder.getDoubleTy(), nullptr, ident);
}

llvm::Type *Codegen::generateType(Type type){
  if(type.kind == Type::Kind::Number)
    return builder.getDoubleTy();
  
  return builder.getVoidTy();
}

llvm::Value *Codegen::generateCallExpr(const ResolvedCallExpr &expr){
  llvm::Function *callee = module.getFunction(expr.callee->ident);

  std::vector<llvm::Value *> args;
  for(auto &&arg: expr.arguments)
    args.emplace_back(generateExpr(*arg));
  
  return builder.CreateCall(callee, args);
}

llvm::Value *Codegen::generateExpr(const ResolvedExpr &expr){
  if(auto *number = dynamic_cast<const ResolvedNumberLiteral *>(&expr))
    return llvm::ConstantFP::get(builder.getDoubleTy(), number->value);
  
  if(auto *dre = dynamic_cast<const ResolvedDeclRefExpr *>(&expr))
    return builder.CreateLoad(builder.getDoubleTy(), declarations[dre->decl]);
    
  if(auto *call = dynamic_cast<const ResolvedCallExpr *>(&expr))
    return generateCallExpr(*call);
  
  if(auto *bin = dynamic_cast<const ResolvedBinaryOperator *>(&expr))
    return generateBinaryOperator(*bin);
  
  if(auto *op = dynamic_cast<const ResolvedUnaryOperator *>(&expr))
    return generateUnaryOperator(*op);
  
  if(auto *g = dynamic_cast<const ResolvedGroupingExpr *>(&expr))
    return generateExpr(*g->expr);
  
  llvm_unreachable("Invalid Expr");
}

llvm::Value *Codegen::generateUnaryOperator(const ResolvedUnaryOperator &op){
  llvm::Value *rhs = generateExpr(*op.operand);

  if(op.op == TokenKind::Minus)
    return builder.CreateFNeg(rhs);
  
  if(op.op == TokenKind::Excl)
    return boolToDouble(builder.CreateNot(doubleToBool(rhs)));
  
  llvm_unreachable("unknown unary op");
  return nullptr;
}

llvm::Function *Codegen::getCurrentFunction(){
  return builder.GetInsertBlock()->getParent();
}

void Codegen::generateConditionalOperator(const ResolvedExpr &op,
llvm::BasicBlock *trueBB, llvm::BasicBlock *falseBB){
  llvm::Function *currentFunction = getCurrentFunction();

  const auto *binop = dynamic_cast<const ResolvedBinaryOperator *>(&op);

  if(binop && binop->op == TokenKind::PipePipe){
    llvm::BasicBlock *nextBB = 
      llvm::BasicBlock::Create(context, "or.lhs.false", currentFunction);

      generateConditionalOperator(*binop->lhs, trueBB, nextBB);
      builder.SetInsertPoint(nextBB);
      generateConditionalOperator(*binop->rhs, trueBB, falseBB);

    return;
  }

  if(binop && binop->op == TokenKind::AmpAmp){
    llvm::BasicBlock *nextBB = 
      llvm::BasicBlock::Create(context, "or.lhs.true", currentFunction);

      generateConditionalOperator(*binop->lhs, nextBB, falseBB);
      builder.SetInsertPoint(nextBB);
      generateConditionalOperator(*binop->rhs, trueBB, falseBB);

    return;
  }
}

llvm::Value *Codegen::generateBinaryOperator(const ResolvedBinaryOperator &bin){
  TokenKind op = bin.op;
  
  llvm::Value *lhs = generateExpr(*bin.lhs);
  llvm::Value *rhs = generateExpr(*bin.rhs);

  if(op == TokenKind::Plus)
    return builder.CreateFAdd(lhs, rhs);
  if(op == TokenKind::Minus)
    return builder.CreateFSub(lhs, rhs);
  if(op == TokenKind::Asterisk)
    return builder.CreateFMul(lhs, rhs);
  if(op == TokenKind::Slash)
    return builder.CreateFDiv(lhs, rhs);
  if(op == TokenKind::Lt)
    return boolToDouble(builder.CreateFCmpOLT(lhs, rhs));
  if(op == TokenKind::Gt)
    return boolToDouble(builder.CreateFCmpOGT(lhs, rhs));
  if(op == TokenKind::EqEq)
    return boolToDouble(builder.CreateFCmpOEQ(lhs, rhs));
  if(op == TokenKind::Leq)
    return boolToDouble(builder.CreateFCmpOLE(lhs, rhs));
  if(op == TokenKind::Geq)
    return boolToDouble(builder.CreateFCmpOGE(lhs, rhs));
  
  llvm_unreachable("unexpected binary op");
  return nullptr;
}

llvm::Value *Codegen::generateReturnStmt(const ResolvedReturnStmt &stmt){
  if(stmt.expr)
    builder.CreateStore(generateExpr(*stmt.expr), retVal);
  
  return builder.CreateBr(retBB);
}

llvm::Value *Codegen::generateStmt(const ResolvedStmt &stmt){
  if(auto *expr = dynamic_cast<const ResolvedExpr *>(&stmt))
    return generateExpr(*expr);
  
  if(auto *returnStmt = dynamic_cast<const ResolvedReturnStmt *>(&stmt))
    return generateReturnStmt(*returnStmt);
  
  llvm_unreachable("unknown statement");
}

void Codegen::generateBlock(const ResolvedBlock &block){
  for(auto &&stmt: block.statements){
    generateStmt(*stmt);

    if(dynamic_cast<const ResolvedReturnStmt *>(stmt.get())){
      builder.ClearInsertionPoint();
      break;
    }
  }
}

void Codegen::generateFunctionBody(const ResolvedFunctionDecl &functionDecl) {
  auto *function = module.getFunction(functionDecl.ident);
  auto *entryBB = llvm::BasicBlock::Create(context, "entry", function);
  builder.SetInsertPoint(entryBB);
  llvm::Value *undef = llvm::UndefValue::get(builder.getInt32Ty());
  allocaInsertPoint = new llvm::BitCastInst(undef, undef->getType(), "alloca.placeholder", entryBB);
  bool isVoid = functionDecl.type.kind != Type::Kind::Number;
  if(!isVoid)
    retVal = allocateStackVariable(function, "retVal");
  // retBB->insertInto(function);

  if(!isVoid)
    retBB = llvm::BasicBlock::Create(context, "return", function);
  
  int idx = 0;
  for(auto &&arg: function->args()){
    const auto *paramDecl = functionDecl.params[idx].get();
    arg.setName(paramDecl->ident);
    llvm::Value *var = allocateStackVariable(function, paramDecl->ident);
    builder.CreateStore(&arg, var);
    declarations[paramDecl] = var;

    ++idx;
  }

  if(functionDecl.ident == "println")
    generateBuiltinPrintBody(functionDecl);
  else
    generateBlock(*functionDecl.body);
  allocaInsertPoint->eraseFromParent();
  allocaInsertPoint = nullptr;

  if (!isVoid && retBB && !retBB->use_empty()) {
    builder.SetInsertPoint(retBB);
    builder.CreateRet(builder.CreateLoad(builder.getDoubleTy(), retVal));
  } else if (isVoid && builder.GetInsertBlock() && builder.GetInsertBlock()->getTerminator() == nullptr) {
    builder.CreateRetVoid();
  }
  // builder.CreateRet(builder.CreateLoad(builder.getDoubleTy(), retVal));
}

void Codegen::generateFunctionDecl(const ResolvedFunctionDecl &functionDecl) {
  auto *retType = generateType(functionDecl.type);
  // functionDecl.dump(0);
  std::vector<llvm::Type *> paramTypes;
  for(auto &&param: functionDecl.params)
    paramTypes.emplace_back(generateType(param->type));
  
  auto *type = llvm::FunctionType::get(retType, paramTypes, false); 
  llvm::Function::Create(type, llvm::Function::ExternalLinkage, functionDecl.ident, module);
}

llvm::Module *Codegen::generateIR() {

  for(auto &&function: resolvedAST){
    generateFunctionDecl(*function);
  }

  for(auto &&function: resolvedAST){
    generateFunctionBody(*function);
  }
  
  generateMainWrapper();

  return &module;
}
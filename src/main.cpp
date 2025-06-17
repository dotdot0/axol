#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "../include/codegen.h"

int main(int argc, char *argv[]){

  std::ifstream inp(argv[1]);
  
  if(!inp) std::cerr << "No input files";

  std::string buf;
  std::string line;

  while(std::getline(inp, buf)) line += buf+"\n";

  Lexer lex(argv[1], line);

  std::cout << "Pipeline Output: " << "\n";

  std::cout << "Parser Output: -> "<< "\n";

  Parser parser(lex);
  auto functions = parser.parseSourceFile();
  for(auto &&function: functions.first){
    function->dump();
  }

  std::cout << "=============" << "\n";

  std::cout << "Sema Output: ->" << "\n";

  Sema sema(std::move(functions.first));
  auto functionsResolved = sema.resolveAST();
  for(auto &&function: functionsResolved){
    function->dump();
  }

  Codegen codegen(std::move(functionsResolved), argv[1]);

  llvm::Module *llvmIr = codegen.generateIR();

  llvmIr->print(llvm::errs(), nullptr);

  std::stringstream path;
  path << "tmp-" << std::filesystem::hash_value(argv[1]) << ".ll";
  std::error_code error_code;
  llvm::raw_fd_ostream f(path.str(), error_code);
  llvmIr->print(f, nullptr);
  std::stringstream command;
  command << "clang " << path.str();
  int ret = std::system(command.str().c_str());
  std::filesystem::remove(path.str());
  return ret;
}

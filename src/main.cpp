#include <iostream>
#include <fstream>
#include "../include/parser.h"
#include "../include/sema.h"

int main(int argc, char *argv[]){

  std::ifstream inp;
  
  inp.open(argv[1]);

  if(!inp) std::cerr << "No input files";

  std::string buf;
  std::string line;

  while(std::getline(inp, buf)) line += buf+"\n";

  Lexer lex(argv[1], line);

  Parser parser(lex);
  auto functions = parser.parseSourceFile();
  for(auto &fn: functions.first){
    fn->dump();
  }
  std::cout << "================" << "\n";

  Sema sema(std::move(functions.first));
  auto functionsResolved = sema.resolveAST();
  for(auto &function: functionsResolved){
    function->dump(0);
  }
  
}

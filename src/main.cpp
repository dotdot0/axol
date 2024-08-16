#include <iostream>
#include <fstream>
#include "parser.h"

int main(int argc, char *argv[]){
  std::ifstream inp;
  inp.open(argv[1]);
  if(!inp) std::cerr << "No input files";
  std::string buf;
  std::string line;
  while(std::getline(inp, buf)) line += buf+"\n";
  Lexer lex(argv[1], line);
  while(lex.idx < line.length()){
    std::cout << lex.getNextToken().to_string() << std::endl;
  }
  // Parser parser(lex);
  // auto functions = parser.parseSourceFile();
  // for(auto &fn: functions.first){
  //   fn->dump();
  // }
}

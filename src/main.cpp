#include <iostream>
#include <fstream>
#include "lexer.h"

int main(int argc, char *argv[]){
  std::ifstream inp;
  inp.open(argv[1]);
  if(!inp) std::cerr << "";
  std::string buf;
  std::string line;
  while(std::getline(inp, buf)) line += buf+"\n";
  Lexer lex(argv[1], line);
  std::cout << line << std::endl;
  while(lex.idx < line.length())
    std::cout << lex.getNextToken().to_string() << std::endl; 
}

#ifndef TYPE_H

#include <string>

struct Type{
  enum class Kind{ Void, Number, Custom };

  Kind kind;
  std::string name;

  static Type builtinVoid() { return {Kind::Void, "void"}; }
  static Type builtinNumber() { return {Kind::Number, "Number"}; }
  static Type custom() { return {Kind::Custom, "Custom"}; }

  private:
    Type(Kind kind, std::string name) : kind(kind), name(std::move(name)){};
};

#endif
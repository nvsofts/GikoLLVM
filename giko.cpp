#include <iostream>
#include <string>
#include <vector>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include "parser.hpp"
#include "generator.hpp"

int main(int argc, char *argv[])
{
  using namespace giko;
  using namespace boost::spirit;

  std::string temp;
  std::string input;

  while (std::getline(std::cin, temp)) {
    input += temp;
    input += '\n';
  }

  std::cout << "Input:" << std::endl;
  std::cout << input << std::endl;

  parser::giko_grammar<std::string::iterator, qi::standard_wide::space_type> g;
  ast::ModuleAST *result = nullptr;
  auto it = input.begin();

  bool success = qi::phrase_parse(it, input.end(), g, qi::standard_wide::space, result);

  if (success && it == input.end()) {
    std::cout << "OK" << std::endl;

    using namespace llvm;

    generator::generator gen;

    std::string error;
    raw_fd_ostream raw_stream("out.bc", error, sys::fs::OpenFlags::F_RW);
    WriteBitcodeToFile(gen.generateModule(result), raw_stream);
    raw_stream.close();

    delete result;
  }else{
    std::cout << "ERROR" << std::endl;
  }

  return 0;
}

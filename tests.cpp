#include "test-lib/test-lib.hpp"
#include "parser.hpp"
#include <string>
#include <exception>
#include <random>
#include <iostream>
#include <sstream>

#define ASCII_MIN 32
#define ASCII_MAX 127

std::string generateRandomString(unsigned int length){
  std::random_device rd;
  std::mt19937 gen(rd());
  std::string randomGeneratedString;
  randomGeneratedString.reserve(length);
  for(unsigned int i = 0; i < length; i++){
    randomGeneratedString += static_cast<char>(
      std::uniform_int_distribution(ASCII_MIN, ASCII_MAX)(gen)
    );
  }
  return randomGeneratedString;
}

std::string randomArgument(unsigned int argNameLength){
  std::string argName;
  if (argNameLength == 1)
    argName = "-";
  else 
    argName = "--";
  return argName + generateRandomString(argNameLength);
}

int main(){
  Test::TestCase seqTest("Named Argument Tests");
  ArgParse::Parser parser;
  seqTest.addTest("Basic Argument Insertion", 
    [&parser](){
      parser.reset();
      parser.addArgument("-a", "Some help text");
      parser.addArgument("--name", "Another help text");
      int argc = 5;
      char* argv[] = { "some_exec", "-a", "yay", "--name", "nay" };
      parser.parse(argc, argv);
      const auto& a = parser.get("a");
      const auto& name = parser.get('name');
      if (a != "yay")
        throw "a argument should be equal to yay but got " + a;
      if (name != "nay")
        throw "name argument should be equal to nay but got " + name;
    }
  );
  seqTest.addTest("Invalid Argument insertion", 
    [&parser](){
      parser.reset();
      parser.addArgument("-a", "Some help text");
      try{
        parser.addArgument("-ne", "Another help text");
      }
      catch (const ArgParse::InvalidKey& e){
        
      }
      catch(const std::exception& e){
        std::stringstream stream;
        stream << e.what();
        stream << " caught, but expected ArgParse::InvalidKey exception";
        throw stream.str();
      }
      throw "-ne is an invalid argument, exception should have been thrown";
    }
  );
  seqTest.addTest("Invalid Argument parsing", 
    [&parser](){
      parser.reset();
      parser.addArgument("-a", "Some help text");
      parser.addArgument("--name", "Another help text");
      int argc = 5;
      char* argv[] = { "some_exec", "-a", "yay", "--invalid_arg", "nay" };
      try{
        parser.parse(argc, argv, false, false);
      }
      catch (const ArgParse::ParserError& e){

      }
      catch(const std::exception& e){
        std::stringstream stream;
        stream << e.what();
        stream << " caught, but expected ArgParse::ParserError exception";
        throw stream.str();
      }
    }
  );
  seqTest.runAll();
}
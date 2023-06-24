# ARGPLUSPLUS
Command Line argument parsing tool to parse command line arguments in C++. Currently keyword arguments support are the best and is hence recommended when using the library. A simple example of using this library is : 
```C++
#include <argplusplus/parser.hpp>
#include <filesystem>

int main(int argc, char** argv){
  ArgParse::Parser parser;
  parser.addArgument("-f", "Single Character keyword argument example");
  parser.addArgument("--file", "Multi Character keyword argument example");
  parser.parse(argc, argv);

  auto fpath = parser.get<std::filesystem::path>("f"); // fpath now have type std::filesystem::path
}
```

## Usage Documentation
```C++
  addArgument(
    std::string&& keyName, // The name of the keyword argument
    std::string&& helpString = "", // The helpstring to display when --help is called
    bool required = true, // if this argument is required
    bool many = false, // if multiple entries to this argument is allowed
    std::string&& defaultValue = "" // the default value of this argument
  )
  parse(
    int argc,
    char** argv,
    bool exitOrException = true, // true : exit when argument parsing encounters error else throw exception
    bool printHelp = true // if help should be printed when an argument parsing error happens
  )
  template<typename T>
  T get<T>(const std::string& keyName);
  // Converts to the target type T, requires that the target type can be constructed from a single std::string argument.
  // Default implementations for built-in types (int, float, long, double) has been given and conversion is done with
  // std::atoi, std::atof
  template<typename T>
  std::vector<T> get<T>(const std::string& keyName, char sep);
  // Converts to a vector of data, sep should always be a ','. The target type is given by T.
```

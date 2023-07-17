#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <exception>
#include <iostream>

#define ARG_SEPARATOR ','

namespace ArgParse{
  /**
   * @brief this will be thrown when a parser error is thrown. This class is 
   * only an inteface for specific parser errors. Use this to catch most parse
   * errors
  */
  class ParserError : public std::exception{
    public:
      virtual const char* what() const noexcept;
  };
  /**
   * @brief thrown for miscellanenous uncategorized errors or unexpected errors
   * that can happen when parsing the content of argv. 
  */
  class GenericParserError : public ParserError{
    public:
      GenericParserError(std::string&& msg);
      const char* what() const noexcept;
    private:
      std::string _msg;
  };
  /**
   * @brief Thrown to print out help message
  */
  class PrintHelp{};
  /**
   * @brief Thrown when an invalid key is found in the parser
  */
  class InvalidKey : public ParserError{
    public:
      InvalidKey(const std::string& key);
      const char* what() const noexcept;
    private:
      std::string _msg;
  };
  /**
   * @brief thrown when a key requested is not in the parsed results.
  */
  class OutOfBounds : public ParserError{
    public:
      OutOfBounds(size_t pos);
      OutOfBounds(const std::string& key);
      const char* what() const noexcept;
    private:
      std::string _msg;
  };
  /**
   * @brief an internal data structure to handle the data keps in Arguments. 
   * This structure specifically only keeps the data. 
  */
  class ArgsData{
    public:
      void set(const std::string& data);
      const std::string& get() const;
      void append(const std::string& data);
      std::vector<std::string> splitBySeparator(char sep) const;
      void clear();
    private:
      std::string _data;
      bool _isInitialized = false;
  };
  /**
   * @brief a data structure that holds the data structure to keep the data for 
   * arguments.
  */
  class Args{
    public:
      Args(
        std::string&& helpString = "", bool required = true, bool many = false, 
        std::string&& defaultValue = ""
      );
      const std::string& data() const;
      const std::string& defaultValue() const;
      const std::string& helpString() const;
      bool isMultiple() const;
      bool isRequired() const;
      bool isInitialized() const;
      void appendOrSet(const std::string& data);
      void clear();

      // Split out
      template<typename Target>
      std::vector<Target> convert(char sep) const;
      template<typename Target>
      Target convert() const;
    private:
      std::string _helpString;
      std::string _default;
      ArgsData _data;
      bool _required;
      bool _many;
  };

  /**
   * @brief a parser that can be used to parse arguments given from the command
   * line. Call parse(argc, argv, ...) to parse arguments given from the command
   * line
  */
  class Parser{
    using Key = std::string;
    public:
      /* Adding arguments */
      template<typename... T>
      /**
       * @brief adds a sequential argument to the parser
      */
      void addSeqArgument(T&& ...args);
      template<typename... T>
      /**
       * @brief adds a keyed argument to the parser
       * @param key the key to use, including "--", for example : "--file", 
       * this decision to include "--" is purely for ease of use
      */
      void addArgument(Key&& key, T&& ...args);
      void getHelpString(std::ostream& stream) const;
      /**
       * @brief Parses the contents of argv into the parser
       * @param argc argument count received from the main function
       * @param argv argument values received from the main function
       * @param exitOrException if the function should exit immediately or throw
       * and exception when any parsing error happens. True is to exit directly
       * @param printHelp if the help message should be printed when any error 
       * is encountered before the function exits or an exception is thrown. If
       * --help or -h is invoked this option will be ignored and the help will
       * be printed anyway and exit will be invoked.
      */
      void parse(
        int argc, char** argv, bool exitOrException = true,bool printHelp = true
      );
      
      /*
        Obtaining the content with conversion
      */
      const std::string& get(size_t pos) const;
      const std::string& get(const Key& key) const;
      template<typename T>
      T get(size_t pos) const;
      template<typename T>
      T get(const Key& key) const;
      template<typename T>
      std::vector<T> get(size_t pos, char sep) const;
      template<typename T>
      std::vector<T> get(const Key& key, char sep) const;

      /*
        Helper Functions
      */
      static bool isValidKey(const Key& key);
      static void checkForHelpArgv(int argc, char** argv);
      static bool isKwargTag(const std::string& s);

      /* 
        Non Static Helper
      */
      bool validateKey(const Key& key) const;
      bool doesKeyExist(const Key& key) const;
      bool doesPosExist(size_t pos) const;

      /*
        Operation on the parser itself
      */
      void reset(bool keepArg = true);
      
    private:
      std::map<Key, Args> _kwargs;
      std::vector<Args> _args;
      bool _parsed = false;

      void _parse(int argc, char** argv);
      void _keyExistOrException(const Key& key) const;
      void _posExistOrException(size_t pos) const;
      void _parsedOrException() const;
      Key _keyNameFromKey(const Key& key) const;
      void _parseArgv(int argc, char** argv);
      void _reset();
      void _resetKeepArgument();
  };
  template<typename T>
  inline T Args::convert() const{
    return T(_data.get());
  }
  template<typename T>
  inline std::vector<T> Args::convert(char sep) const{
    std::vector<T> converted;
    auto separated = _data.splitBySeparator(sep);
    converted.reserve(separated.size());
    std::transform(
      separated.begin(), separated.end(), std::back_inserter(converted), 
      [](const std::string& s){ return T(s);}
    );
    return converted;
  }

  // Parser
  template<typename... T>
  inline void Parser::addSeqArgument(T&& ...args){
    _args.emplace_back(std::forward<T>(args)...);
  }
  template<typename ...T>
  inline void Parser::addArgument(Key&& key, T&& ...args){
    if (!validateKey(key))
      throw InvalidKey(key);
    _kwargs.emplace(_keyNameFromKey(key), Args(std::forward<T>(args)...));
  }
  template<typename T>
  inline T Parser::get(size_t pos) const{
    _parsedOrException();
    _posExistOrException(pos);
    return _args.at(pos).convert<T>();
  }
  template<typename T>
  inline T Parser::get(const std::string& key) const{
    _parsedOrException();
    _keyExistOrException(key);
    return _kwargs.at(key).convert<T>();
  }
  template<typename T>
  inline std::vector<T> Parser::get(size_t pos, char sep) const{
    _parsedOrException();
    _posExistOrException(pos);
    const Args& arg = _args.at(pos);
    return arg.convert<T>(sep);
  }
  template<typename T>
  inline std::vector<T> Parser::get(const Key& key, char sep) const{
    _parsedOrException();
    _keyExistOrException(key);
    const Args& arg = _kwargs.at(key);
    return arg.convert<T>(sep);
  }
  template<>
  inline double Args::convert<double>() const{
    return std::atof(_data.get().c_str());
  }
  template<>
  inline int Args::convert<int>() const{
    return std::atoi(_data.get().c_str());
  }
  template<>
  inline bool Args::convert<bool>() const{
    char firstCharacter = _data.get().at(0);
    return firstCharacter == 't' || firstCharacter == 'T';
  }
  template<>
  inline uint32_t Args::convert<uint32_t>() const{
    return static_cast<uint32_t>(convert<int>());
  }
  template<>
  inline size_t Args::convert<size_t>() const{
    return static_cast<size_t>(convert<int>());
  }
  template<>
  inline float Args::convert<float>() const{
    return static_cast<float>(convert<double>());
  }
  template<>
  inline std::vector<double> Args::convert<double>(char sep) const{
    std::vector<double> converted;
    auto separated = _data.splitBySeparator(sep);
    converted.reserve(separated.size());
    std::transform(
      separated.begin(), separated.end(), std::back_inserter(converted), 
      [](const std::string& s){
        return std::atof(s.c_str());
      }
    );
    return converted;
  }
  template<>
  inline std::vector<int> Args::convert<int>(char sep) const{
    std::vector<int> converted;
    auto separated = _data.splitBySeparator(sep);
    converted.reserve(separated.size());
    std::transform(
      separated.begin(), separated.end(), std::back_inserter(converted), 
      [](const std::string& s){
        return std::atoi(s.c_str());
      }
    );
    return converted;
  }
  template<>
  inline std::vector<bool> Args::convert<bool>(char sep) const{
    std::vector<bool> converted;
    auto separated = _data.splitBySeparator(sep);
    converted.reserve(separated.size());
    std::transform(
      separated.begin(), separated.end(), std::back_inserter(converted), 
      [](const std::string& s){
        char firstCharacter = s.at(0);
        return firstCharacter == 't' || firstCharacter == 'T';
      }
    );
    return converted;
  }
  template<>
  inline std::vector<uint32_t> Args::convert<uint32_t>(char sep) const{
    std::vector<uint32_t> converted;
    auto separated = _data.splitBySeparator(sep);
    converted.reserve(separated.size());
    std::transform(
      separated.begin(), separated.end(), std::back_inserter(converted), 
      [](const std::string& s){
        return static_cast<uint32_t>(std::atoi(s.c_str()));
      }
    );
    return converted;
  }
  template<>
  inline std::vector<size_t> Args::convert<size_t>(char sep) const{
    std::vector<size_t> converted;
    auto separated = _data.splitBySeparator(sep);
    converted.reserve(separated.size());
    std::transform(
      separated.begin(), separated.end(), std::back_inserter(converted), 
      [](const std::string& s){
        return static_cast<size_t>(std::atoi(s.c_str()));
      }
    );
    return converted;
  }
  template<>
  inline std::vector<float> Args::convert<float>(char sep) const{
    std::vector<float> converted;
    auto separated = _data.splitBySeparator(sep);
    converted.reserve(separated.size());
    std::transform(
      separated.begin(), separated.end(), std::back_inserter(converted), 
      [](const std::string& s){
        return static_cast<float>(std::atof(s.c_str()));
      }
    );
    return converted;
  }
}

#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <memory>

namespace ArgParse{
  // Helper Class that is subject to change
  class Args{
    public:
      Args(
        std::string&& helpString = "",
        bool required = true,
        bool many = false
      ){
        _helpString = std::move(helpString);
        _bulk       = many;
        _required   = required;
      }
      const std::string& data() const{
        return _data[0];
      }
      const std::vector<std::string>& allData() const{
        return _data;
      }
    private:
      bool _bulk;
      bool _required;
      std::vector<std::string> _data;
      std::string _helpString;

      friend class Parser;
  };
  class Parser{
    public:
      void printHelp() const{
        std::cout<<"Ordered Arguments List : "<<std::endl;
        for(const auto& entry : _args)
          std::cout<<"\t"<<entry._helpString<<std::endl;
        std::cout<<"Keyword arguments List : "<<std::endl;
        for(const auto& [key, entry] : _kwargs)
          std::cout<<"\t--"<<key<<":"<<entry._helpString<<std::endl;
      }
      void addArgument(Args&& args){
        _args.emplace_back(args);
      }
      void addArgument(const std::string& name, Args&& args){
        if (_kwargs.count(name) != 0)
          throw "Key already exists";
        if (name == "h")
          throw "h is a forbidden name";
        if (name.find("-") != std::string::npos)
          throw "- should not appear in the name";
        // Add into the map
        _kwargs.emplace(name, args);
      }
      void parse(int argc, char** argv){
        // Check for -h and print help if exists
        for(size_t i = 1; i < argc; i++){
          if(argv[i] == "-h"){
            printHelp();
            return;
          }
        }
        // Real Parsing
        size_t argCount   = 0;
        bool haveKey      = false;
        std::string curArg= "";
        for (size_t i = 1; i < argc; i++){
          std::string arg(argv[i]);
          bool isKwargStart = arg.substr(0, 2) == "--";
          if (isKwargStart && !haveKey){
            if (_kwargs.count(arg.substr(2)) == 0){
              std::cerr<<"No such key "<< arg << std::endl;
              std::cerr<<"Call " << argv[0] << " -h for help";
              throw "Error Parsing";
            }
            curArg  = arg.substr(2);
            haveKey = true;
          }
          // Add Kwargs Argument
          else if (!isKwargStart && haveKey){
            auto& args  = _kwargs[curArg];
            if (!args._bulk && args._data.size() != 0)
              throw curArg + " have too many arguments";
            args._data.push_back(argv[i]);
            haveKey = false;
          }
          // Add Args Argument
          else if(!isKwargStart && !haveKey){
            if (argCount > _args.size())
              throw "Too many ordered arguments";
            _args[argCount]._data.push_back(argv[i]);
            argCount += 1;
          }
          else
            throw "Error Parsing";
        }
        if (haveKey){
          std::cerr<<"Key " << curArg << " have no value";
          std::cerr<<"Call "<< argv[0] << " -h for help";
          throw "Error Parsing";
        }
        // Check requireds
        for (const auto& entry : _args){
          if (entry._required && entry._data.size() == 0){
            std::cerr<<"Ordered Arguments are incomplete";
          }
        }
        for (const auto& [key, entry] : _kwargs){
          if (entry._required && entry._data.size() == 0){
            std::cerr<<"Named Arguments for key " << key <<" have not been given";
          }
        }
        _parsed = true;
      }
      const Args& getKwargs(const std::string& key) const{
        if (_parsed == false){
          throw "Parsing have not been done";
        }
        if (_kwargs.count(key) == 0)
          throw "No such name";
        
      }
      const Args& getArgs(size_t args) const{
        if (_parsed == false){
          throw "Parsing have not been done";
        }
      }
    private:
      std::map<std::string, Args> _kwargs;
      std::vector<Args> _args;
      bool _parsed = false;
  };
};
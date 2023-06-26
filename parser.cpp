#include "parser.hpp"

namespace ArgParse{
  const char* ParserError::what() const noexcept{
    return "Unknown parsing error";
  }
  InvalidKey::InvalidKey(const std::string& key){
    _msg = "The following key \'" + key + "\' is invalid";
  }
  const char* InvalidKey::what() const noexcept{
    return _msg.c_str();
  }
  GenericParserError::GenericParserError(std::string&& msg){
    _msg = std::move(msg);
  }
  const char* GenericParserError::what() const noexcept{
    return _msg.c_str();
  }
  OutOfBounds::OutOfBounds(size_t pos){
    _msg = "The following position " + std::to_string(pos) + 
      " is not in the parser";
  }
  OutOfBounds::OutOfBounds(const std::string& key){
    _msg = "The following key \'" + key + "\' does not exist in the parser";
  }
  const char* OutOfBounds::what() const noexcept{
    return _msg.c_str();
  }
  void ArgsData::setData(const std::string& data){
    if (!_isInitialized) 
      _isInitialized = true;
    _data = data;
  }
  const std::string& ArgsData::getData() const{
    return _data;
  }
  void ArgsData::appendData(const std::string& data){
    _data = _data.size() == 0 ? data : _data + ARG_SEPARATOR + data;
  }
  std::vector<std::string> ArgsData::splitBySeparator(char sep) const{
    // Copy Internal Data before using it
    std::string data = _data;
    data.reserve(_data.size() + 1);
    data.push_back(sep);
    // Separate it
    std::vector<std::string> separated;
    size_t entryCount = std::count(data.begin(), data.end(), sep);
    separated.reserve(entryCount + 1);
    // Actually Split it
    size_t begPointer = 0;
    size_t endPointer = 0;
    for(endPointer = 0; endPointer < data.size(); endPointer++){
      if (data.at(endPointer) == sep){
        separated.push_back(
          std::string(data.begin() + begPointer, data.begin() + endPointer)
        );
        begPointer = endPointer + 1;
      }
    }
    return separated;
  }
  Args::Args(
    std::string&& helpString, bool required, bool many, 
    std::string&& defaultValue
  ){
    _helpString = std::move(helpString);
    _default    = std::move(defaultValue);
    _required   = required;
    _many       = many;
  }
  const std::string& Args::data() const{
    return _data.getData();
  }
  const std::string& Args::defaultValue() const{
    return _default;
  }
  const std::string& Args::helpString() const{
    return _helpString;
  }
  bool Args::isInitialized() const{
    return _data.getData().size() != 0;
  }
  bool Args::isMultiple() const{
    return _many;
  }
  bool Args::isRequired() const{
    return _required;
  }
  void Args::appendOrSetData(const std::string& data){
    if(isMultiple()) 
      _data.appendData(data);
    else
      _data.setData(data);
  }
  bool Parser::doesKeyExist(const Key& key) const{
    return _kwargs.count(key) != 0;
  }
  void Parser::_keyExistOrException(const Key& key) const{
    if (!doesKeyExist(key))
      throw OutOfBounds(key);
  }
  bool Parser::doesPosExist(size_t pos) const{
    return pos < _args.size();
  }
  void Parser::_posExistOrException(size_t pos) const{
    if (!doesPosExist(pos))
      throw OutOfBounds(pos);
  }
  void Parser::_parsedOrException() const{
    if (!_parsed)
      throw GenericParserError("Parsing have not been done");
  }
  bool Parser::isValidKey(const Key& key){
    return key != "-h" && key != "--help" && isKwargTag(key);
  }
  bool Parser::validateKey(const std::string& key) const{
    return isValidKey(key) && !doesKeyExist(key);
  }
  std::string Parser::_keyNameFromKey(const std::string& key) const{
    if (key.length() == 2) 
      return std::string(key.begin() + 1, key.end());
    else if (key.length() > 2)
      return std::string(key.begin() + 2, key.end());
    else
      throw InvalidKey(key);
  }
  const std::string& Parser::get(size_t pos) const{
    _parsedOrException();
    _posExistOrException(pos);
    return _args.at(pos).data();
  }
  const std::string& Parser::get(const std::string& key) const{
    _parsedOrException();
    _keyExistOrException(key);
    return _kwargs.at(key).data();
  }
  void Parser::getHelpString(std::ostream& stream) const{
    stream << "Ordered Arguments List : \n";
    for (const auto& entry : _args){
      stream << "\t" << entry.helpString();
      stream <<  " default : " << entry.defaultValue() << "\n";
    }
    stream << "Keyword Arguments List : \n";
    for(const auto& [key, entry] : _kwargs){
      if (key.length() == 1) stream << "\t-" << key;
      else stream << "\t--" << key;
      stream << "\t\t : "<< entry.helpString();
      stream <<  " default : " << entry.defaultValue() << "\n";
    }
  }
  void Parser::parse(
    int argc, char** argv, bool exitOnFail, bool printHelp
  ){
    if (_parsed)
      throw GenericParserError("Content have been parsed");
    try{
      _parse(argc, argv);
      _parsed = true;
    }
    catch(const ParserError& e){
      if (exitOnFail){
        std::cerr<< e.what() << std::endl;
        if (printHelp) getHelpString(std::cerr);
        exit(1);
      }
      else{
        if (printHelp) getHelpString(std::cerr);
        throw e;
      }
    }
    catch(const PrintHelp& e){
      getHelpString(std::cerr);
      exit(1);
    }
  }
  void Parser::checkForHelpArgv(int argc, char** argv){
    for(size_t i = 1; i < argc; i++){
      if (argv[i] == std::string("-h") || argv[i] == std::string("--help"))
        throw PrintHelp();
    }
  }
  bool Parser::isKwargTag(const std::string& key){
    return key.length() > 1 && ((
      key.length() == 2 && key.at(0) == '-'
    ) || (
      key.length() > 2 && key.substr(0, 2) == "--"
    ));
  }
  void Parser::_parseArgv(int argc, char** argv){
    size_t argCount = 0;
    bool haveKey = false;
    std::string curArgKey = "";
    for(size_t i = 1; i < argc; i++){
      std::string arg = argv[i];
      bool isKwargStart = isKwargTag(arg);
      if (isKwargStart && !haveKey){
        if (!doesKeyExist(_keyNameFromKey(arg))){
          throw OutOfBounds(arg);
        }
        curArgKey = _keyNameFromKey(arg);
        haveKey = true;
      }
      else if (!isKwargStart && haveKey){
        _kwargs.at(curArgKey).appendOrSetData(arg);
        haveKey = false;
      }
      else if (!isKwargStart && !haveKey){
        if (argCount > _args.size())
          throw OutOfBounds(argCount);
        _args.at(argCount).appendOrSetData(arg);
        argCount += 1;
      }
      else
        throw ParserError();
    }
    if (haveKey){
      throw GenericParserError("The key \'" + curArgKey + " is uninitialized");
    }
    size_t errCount = 0;
    for(auto& entry : _args){
      if (!entry.isInitialized())
        entry.appendOrSetData(entry.defaultValue());
      if (entry.isRequired() && !entry.isInitialized())
        throw GenericParserError("Ordered Arguments are incomplete");
    }
    for(auto& [key, entry] : _kwargs){
      if (!entry.isInitialized())
        entry.appendOrSetData(entry.defaultValue());
      if (entry.isRequired() && !entry.isInitialized())
        throw GenericParserError(
          "Inordered Argument \'" + key + "\' is not given"
        );
    }
  }
  void Parser::_parse(int argc, char** argv){
    checkForHelpArgv(argc, argv);
    _parseArgv(argc, argv);
  }
};
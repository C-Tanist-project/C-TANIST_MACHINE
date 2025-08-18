#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "types.hpp"

class Linker {
 private:
  struct Module {
    std::string name;
    std::vector<int16_t> code;
    std::unordered_map<std::string, int16_t> intDefTable;
    std::unordered_map<std::string, std::vector<int16_t>> intUseTable;
    std::unordered_map<int16_t, OperandFormat> relocationTable;
    int16_t stackSize = 0;
    int16_t startAddress = 0;
    int16_t loadAddress = 0;
  };

  std::vector<Module> modules;

  std::unordered_map<std::string, std::pair<int16_t, std::string>>
      globalSymbolTable;
  std::vector<std::string> errors;
  int16_t totalStackSize = 0;
  int16_t currentLoadAddress = 0;

  void SecondPass();
  void GenerateOutput(const std::string& outputName);

 public:
  Linker();
  void FirstPass(const std::vector<std::string>& objFilePaths);
  void ReadObjectCodeFile(const std::string& filePath);
  void printModules();
  void Link(const std::vector<std::string>& objFilePaths);
};

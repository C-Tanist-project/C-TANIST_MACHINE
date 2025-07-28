#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "types.hpp"

typedef struct par {
  std::string parameter;
  int level;
  int position;
} macroParameter_t;

typedef struct structMacro {
  std::string macroName;
  std::string macroSkeleton;
  int arguments = 0;
  int expansions = 0;
} Macro_t;

class MacroProcessor {
  int levelCounter;
  bool isExpanding;
  std::stack<macroParameter_t> formalParameters;
  std::vector<Macro_t> definedMacros;
  std::string asmFilePath;

public:
  MacroProcessor(const std::string &asmFilePath);
  void Pass();
};

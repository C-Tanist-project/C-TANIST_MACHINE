#include <algorithm>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

#include "types.hpp"

typedef struct par {
  std::string parameter;
  int level;
  int position;
} MacroParameter_t;

typedef struct structMacro {
  std::string macroName;
  std::string macroSkeleton;
} Macro_t;

class MacroProcessor {
  int levelCounter;
  bool isExpanding;
  std::deque<MacroParameter_t> formmalParameters;
  std::vector<Macro_t> definedMacros;
  std::string asmFilePath;

public:
  MacroProcessor(const std::string &asmFilePath);
  void Pass();
};

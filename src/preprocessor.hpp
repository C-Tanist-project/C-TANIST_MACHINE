#pragma once

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <deque>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>

#include "types.hpp"

typedef struct {
  int level;
  int position;
} ParemeterCoordinates;

typedef struct {
  std::string name;
  ParemeterCoordinates coordinates;
} MacroParameter_t;

typedef struct {
  std::string macroName;
  std::string macroSkeleton;
} Macro_t;

inline std::string CreateSubstitutionPattern(int level, int position);
inline ParemeterCoordinates ReadSubstitutionPattern(std::string input);
std::regex CreateReverseRegex(std::string input);

class MacroProcessor {
  int levelCounter;
  bool isExpanding;
  std::vector<std::string> actualParameters;
  std::deque<MacroParameter_t> formalParameters;
  std::vector<Macro_t *> definedMacros;
  std::string asmFilePath;

  Macro_t *currentMacroDefinition = NULL;
  Macro_t *currentMacroCall = NULL;

  inline void ProcessPrototype(std::string line);
  inline void PushFormalParameter(std::string name, int position);
  Macro_t *SearchDefinedMacros(std::string name);
  void ParseActualParameters(std::string line, std::string macroName);

public:
  MacroProcessor(const std::string &asmFilePath);
  void Pass();
};

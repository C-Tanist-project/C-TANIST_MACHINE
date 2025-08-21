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
  int level;
  std::vector<std::string> actualParameters;
} Expansion_t;

typedef struct {
  std::string macroName;
  std::string macroPrototype;
  std::string macroSkeleton;
} Macro_t;

inline std::string CreateSubstitutionPattern(int level, int position);
inline ParemeterCoordinates ReadSubstitutionPattern(std::string input);

class MacroProcessor {
  int definitionLevel;
  int expansionLevel;
  std::deque<std::unique_ptr<std::stringstream>> inputSourceStack;
  std::deque<Expansion_t> actualParametersStack;
  std::deque<MacroParameter_t> formalParameters;
  std::vector<Macro_t *> definedMacros;

  std::string asmFilePath;
  std::string outputFilePath;

  Macro_t *currentMacroDefinition = NULL;
  Macro_t *currentMacroCall = NULL;
  std::stringstream *currentInputSource = NULL;

  inline void ProcessPrototype(std::string line);
  inline void PushFormalParameter(std::string name, int position);
  Macro_t *SearchDefinedMacros(std::string name);
  std::vector<std::string> GetActualParameterList(std::string line,
                                                  Macro_t *foundMacro);
  std::string ReplaceFormalParameters(std::string line);
  std::string ReplaceSubstitutionPatterns(std::string line);
  void PopFormalParameterLevel(int level);
  void PopActualParameterLevel(int level);

public:
  MacroProcessor();
  void Pass(const std::string &asmFilePath);
};

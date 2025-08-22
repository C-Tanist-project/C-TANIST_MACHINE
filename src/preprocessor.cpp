#include "preprocessor.hpp"

static inline bool IcharEquals(char a, char b) {
  return std::tolower(static_cast<unsigned char>(a)) ==
         std::tolower(static_cast<unsigned char>(b));
}

static inline bool IsEquals(const std::string &a, const std::string &b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), IcharEquals);
}

inline std::string CreateSubstitutionPattern(int level, int position) {
  return std::format("#({},{})", level, position);
}

inline ParemeterCoordinates ReadSubstitutionPattern(std::string input) {
  int internalLevel;
  int internalPosition;
  sscanf(input.c_str(), "#(%d,%d)", &internalLevel, &internalPosition);
  return ParemeterCoordinates{.level = internalLevel,
                              .position = internalPosition};
}

inline void MacroProcessor::PushFormalParameter(std::string name,
                                                int position) {
  MacroParameter_t *newParameter = new MacroParameter_t;

  ParemeterCoordinates *newPosition = new ParemeterCoordinates;

  *newPosition = {.level = this->definitionLevel, .position = position};

  newParameter->name = name;
  newParameter->coordinates = {.level = this->definitionLevel,
                               .position = position};

  this->formalParameters.push_back(*newParameter);
}

inline void MacroProcessor::ProcessPrototype(std::string line) {
  std::stringstream stringStream(line);
  std::string currentToken;
  size_t position = 0;
  bool isNameFound = false;

  while (stringStream >> currentToken) {
    std::stringstream secondaryTokenizer(currentToken);
    std::string currentSecondaryToken;

    while (std::getline(secondaryTokenizer, currentSecondaryToken, ',')) {
      if (currentSecondaryToken[0] == '&') {
        PushFormalParameter(currentSecondaryToken, position);
        position++;
      } else {
        if (!isNameFound) {
          if (this->definitionLevel == 1) {
            currentMacroDefinition->macroName = currentSecondaryToken;
          }
          isNameFound = true;
        } else {
          // só a primeira "coisa" que não é parâmetro é nome de macro (faz
          // sentido, né?)
          continue;
        }
      }
    }
  }
  if (this->definitionLevel == 1) {
    currentMacroDefinition->macroPrototype = line;
  }
}

Macro_t *MacroProcessor::SearchDefinedMacros(std::string line) {
  std::regex regexName(R"([^\s]+)");
  std::smatch matchName;
  std::string macroName;

  auto wordsBegin = std::sregex_iterator(line.begin(), line.end(), regexName);
  auto wordsEnd = std::sregex_iterator();

  std::vector<std::string> tokens;

  for (auto i = wordsBegin; i != wordsEnd; ++i) {
    tokens.push_back((*i).str());
  }

  if (tokens.empty()) {
    return NULL;
  }

  for (auto const &macro : this->definedMacros) {
    if (IsEquals(macro->macroName, tokens[0])) {
      return macro;
    }
  }

  if (tokens.size() > 1) {
    for (auto const &macro : this->definedMacros) {
      if (IsEquals(macro->macroName, tokens[1])) {
        return macro;
      }
    }
  }

  return NULL;
}

std::vector<std::string> MacroProcessor::GetActualParameterList(
    std::string line, Macro_t *foundMacro) {
  std::vector<std::string> currentActualParameters;

  std::string prototype = foundMacro->macroPrototype;
  std::string name = foundMacro->macroName;

  size_t prototypeNamePosition = prototype.find(name);
  size_t lineNamePosition = line.find(name);

  std::string prototypeLeftPart = prototype.substr(0, prototypeNamePosition);

  std::string lineLeftPart = line.substr(0, lineNamePosition);
  std::string lineRightPart = line.substr(lineNamePosition + name.length());

  std::stringstream leftPrototypeStream(prototypeLeftPart);
  std::stringstream leftLineStream(lineLeftPart);
  std::stringstream rightLineStream(lineRightPart);

  std::string currentFormalParameter;

  while (leftPrototypeStream >> currentFormalParameter) {
    std::string currentActualParameter;
    if (!(leftLineStream >> currentActualParameter)) {
      currentActualParameters.push_back("    ");
    } else {
      currentActualParameters.push_back(currentActualParameter);
    }
  }

  std::string currentParameter;
  while (std::getline(rightLineStream, currentParameter, ',')) {
    size_t first = currentParameter.find_first_not_of(" \t");
    if (first == std::string::npos) {
      currentActualParameters.push_back("");
    } else {
      size_t last = currentParameter.find_last_not_of(" \t");
      currentActualParameters.push_back(
          currentParameter.substr(first, (last - first + 1)));
    }
  }

  return currentActualParameters;
}

std::string MacroProcessor::ReplaceFormalParameters(std::string line) {
  std::regex matchFormalParameter("\\&\\w+\\b", std::regex_constants::icase);
  std::sregex_iterator it(line.cbegin(), line.cend(), matchFormalParameter);
  std::sregex_iterator end;

  if (it == end) return line;

  std::string newLine;
  auto lastMatchPosition = line.cbegin();

  for (; it != end; ++it) {
    std::smatch match = *it;
    newLine += match.prefix().str();
    bool foundParameter = false;

    for (auto i = formalParameters.rbegin(); i != formalParameters.rend();
         ++i) {
      if (IsEquals(i->name, match.str())) {
        foundParameter = true;
        newLine += CreateSubstitutionPattern(i->coordinates.level,
                                             i->coordinates.position);
        break;
      }
    }

    if (!foundParameter) {
      newLine += match.str();
    }

    lastMatchPosition = match.suffix().first;
  }

  newLine.append(lastMatchPosition, line.cend());

  return newLine;
}

std::string MacroProcessor::ReplaceSubstitutionPatterns(std::string line) {
  std::regex matchSubstitutionPattern("#\\((\\d+),(\\d+)\\)");
  std::sregex_iterator it(line.cbegin(), line.cend(), matchSubstitutionPattern);
  std::sregex_iterator end;

  std::string newLine;
  auto lastMatchPosition = line.cbegin();

  for (; it != end; ++it) {
    std::smatch match = *it;

    newLine += match.prefix().str();
    ParemeterCoordinates coords = ReadSubstitutionPattern(match.str());

    if (coords.level == 1) {
      if ((size_t)coords.position <
          this->actualParametersStack.back().actualParameters.size()) {
        newLine += this->actualParametersStack.back()
                       .actualParameters[coords.position];
      } else {
        exit(-1);
      }
    } else {
      newLine += CreateSubstitutionPattern(coords.level - 1, coords.position);
    }
    lastMatchPosition = match.suffix().first;
  }
  newLine.append(lastMatchPosition, line.cend());

  return newLine;
}

void MacroProcessor::PopFormalParameterLevel(int level) {
  for (auto i = this->formalParameters.begin();
       i != this->formalParameters.end();) {
    if ((*i).coordinates.level == level && !this->formalParameters.empty()) {
      i = this->formalParameters.erase(i);
    } else {
      i++;
    }
  }
}
void MacroProcessor::PopActualParameterLevel(int level) {
  for (auto i = this->actualParametersStack.begin();
       i != this->actualParametersStack.end();) {
    if ((*i).level == level && !this->actualParametersStack.empty()) {
      i = this->actualParametersStack.erase(i);
    } else {
      i++;
    }
  }
}

MacroProcessor::MacroProcessor(const std::string &asmFilePath) {
  this->asmFilePath = asmFilePath;
  this->outputFilePath = "MASMAPRG.ASM";
  this->definitionLevel = 0;
  this->expansionLevel = 0;
}

void MacroProcessor::Pass() {
  std::ifstream file(asmFilePath);
  std::ofstream output(outputFilePath);

  std::string line;

  std::regex matchMacro("\\bmacro\\b", std::regex_constants::icase);
  std::regex matchMend("\\bmend\\b", std::regex_constants::icase);

  this->actualParametersStack.clear();

  bool nextLineIsPrototype = false;

  if (!file.is_open()) {
    perror("Não foi possível abrir o arquivo");
    exit(-1);
  }

  if (!std::getline(file, line)) {
    perror("Arquivo vazio");
    exit(-1);
  }

  while (true) {
    if (line.empty()) {
      if (this->expansionLevel > 0) {
        if (!std::getline(*(this->inputSourceStack.back().get()), line)) break;
      } else {
        if (!std::getline(file, line)) break;
      }
      continue;
    }

    do {
      if (std::regex_search(line, matchMacro)) {
        nextLineIsPrototype = true;
        this->definitionLevel++;
        if (this->definitionLevel == 1) {
          this->currentMacroDefinition = new Macro_t;
          this->definedMacros.push_back(this->currentMacroDefinition);
        } else {
          this->currentMacroDefinition->macroSkeleton.append(line + '\n');
        }
        continue;
      }

      if (nextLineIsPrototype == true) {
        ProcessPrototype(line);
        if (this->definitionLevel > 0)
          this->currentMacroDefinition->macroSkeleton.append(line + '\n');
        nextLineIsPrototype = false;
        continue;
      }

      Macro_t *foundMacro = SearchDefinedMacros(line);

      if (foundMacro != NULL) {
        if (this->definitionLevel == 0) {
          this->expansionLevel++;
          std::vector<std::string> aps =
              GetActualParameterList(line, foundMacro);
          this->actualParametersStack.push_back(Expansion_t{
              .level = this->expansionLevel, .actualParameters = aps});

          this->currentMacroCall = foundMacro;

          auto newStream = std::make_unique<std::stringstream>();
          newStream->str(foundMacro->macroSkeleton);

          std::string dummy;
          std::getline(*newStream, dummy);

          this->inputSourceStack.push_back(std::move(newStream));
        }
        if (this->definitionLevel > 0 && this->expansionLevel == 0) {
          line = ReplaceFormalParameters(line);
        }
        if (this->definitionLevel > 0) {
          this->currentMacroDefinition->macroSkeleton.append(line + '\n');
        }
        continue;
      }

      if (std::regex_search(line, matchMend)) {
        if (this->definitionLevel == 0) {
          this->inputSourceStack.pop_back();
          PopActualParameterLevel(this->expansionLevel);
          this->expansionLevel--;
        } else {
          if (this->expansionLevel == 0) {
            PopFormalParameterLevel(this->definitionLevel);
          }
          this->currentMacroDefinition->macroSkeleton.append(line + '\n');
          this->definitionLevel--;
        }
        continue;
      }

      if (this->expansionLevel == 0 && this->definitionLevel > 0) {
        line = ReplaceFormalParameters(line);
      }

      if (this->definitionLevel == 0) {
        output << line << std::endl;
      } else {
        this->currentMacroDefinition->macroSkeleton.append(line + '\n');
      }

    } while (false);

    if (this->expansionLevel > 0) {
      if (!std::getline(*(this->inputSourceStack.back().get()), line)) break;
      line = ReplaceSubstitutionPatterns(line);
    } else {
      if (!std::getline(file, line)) break;
    }
  }
  output.close();
}

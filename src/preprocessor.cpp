#include "preprocessor.hpp"

static inline bool IcharEquals(char a, char b) {
  return std::tolower(static_cast<unsigned char>(a)) ==
         std::tolower(static_cast<unsigned char>(b));
}

static inline bool IsEquals(const std::string &a, const std::string &b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), IcharEquals);
}

std::regex CreateReverseRegex(std::string input) {
  return std::regex("^((?!" + input + ").)*");
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
  this->formalParameters.push_back(
      MacroParameter_t{.name = name,
                       .coordinates = ParemeterCoordinates{
                           .level = this->levelCounter, .position = position}});
}

inline void MacroProcessor::ProcessPrototype(std::string line) {
  std::regex matchPrototypeParameter("\\&\\w+\\b", std::regex_constants::icase);
  std::regex matchPrototypeName("^\\s*(?:&\\w+\\s+)?(\\w+).*",
                                std::regex_constants::icase);
  std::smatch matchName;
  std::smatch matchParameter;
  std::string macroName;

  if (std::regex_search(line, matchName, matchPrototypeName)) {
    macroName = matchName[1].str();
  } else {
    throw("MACRO_NAME_NOT_DEFINED");
  }

  auto lineBegin =
      std::sregex_iterator(line.begin(), line.end(), matchPrototypeParameter);

  auto lineEnd = std::sregex_iterator();

  int parameterCount = std::distance(lineBegin, lineEnd);
  int parameterPosition = 0;

  if (parameterCount > 0 && !this->isExpanding) {
    for (std::sregex_iterator i = lineBegin; i != lineEnd; i++) {
      matchParameter = *i;
      parameterPosition++;
      std::string matchString = matchParameter.str();
      PushFormalParameter(matchString, parameterPosition);
    }
  }

  if (this->levelCounter > 1) {
    this->currentMacroDefinition->macroSkeleton.append(line + '\n');
  }

  if (this->currentMacroDefinition != NULL) {
    this->currentMacroDefinition->macroName = macroName;
  }
}

Macro_t *MacroProcessor::SearchDefinedMacros(std::string line) {
  std::regex regexName(R"([^\s]+)");
  std::smatch matchName;
  std::string macroName;

  auto words_begin = std::sregex_iterator(line.begin(), line.end(), regexName);
  auto words_end = std::sregex_iterator();

  std::vector<std::string> tokens;
  for (auto i = words_begin; i != words_end; ++i) {
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

void MacroProcessor::ParseActualParameters(std::string line,
                                           std::string macroName) {
  this->actualParameters.clear();

  std::stringstream streamFromLine(line);

  std::string token;

  while (streamFromLine >> token) {
    std::stringstream streamFromToken(token);
    std::string subtoken;
    while (std::getline(streamFromToken, subtoken, ',')) {
      if (subtoken != macroName) {
        this->actualParameters.push_back(subtoken);
      }
    }
  }
}

MacroProcessor::MacroProcessor(const std::string &asmFilePath) {
  this->isExpanding = false;
  this->asmFilePath = asmFilePath;
  this->levelCounter = 0;
}

void MacroProcessor::Pass() {
  std::ifstream file(asmFilePath);
  std::stringstream expansionStream;
  std::string line;
  std::regex matchMacro("\\bmacro\\b", std::regex_constants::icase);
  std::regex matchMend("\\bmend\\b", std::regex_constants::icase);
  std::regex matchSubstitutionPattern("#\\((\\d+),(\\d+)\\)");
  std::regex matchFormalParameter("\\&\\w+\\b", std::regex_constants::icase);
  this->actualParameters.clear();
  bool nextLineIsPrototype = false;

  if (!file.is_open()) {
    perror("Não foi possível abrir o arquivo");
    exit(-1);
  }

  std::getline(file, line);

  while (!line.empty()) {
    if ((this->currentMacroCall = SearchDefinedMacros(line)) != NULL &&
        !nextLineIsPrototype) {
      this->isExpanding = true;
      this->actualParameters.clear();
      ParseActualParameters(line, currentMacroCall->macroName);
      expansionStream.str(this->currentMacroCall->macroSkeleton);
      expansionStream.clear();
    }

    else if (nextLineIsPrototype && !this->isExpanding) {
      ProcessPrototype(line);
      nextLineIsPrototype = false;
    }

    else if (std::regex_search(line, matchMacro)) {
      this->levelCounter++;
      if (levelCounter == 1) {
        Macro_t *newMacro = new Macro_t;
        this->currentMacroDefinition = newMacro;
        this->definedMacros.push_back(newMacro);
      } else {
        if (this->currentMacroDefinition != NULL) {
          this->currentMacroDefinition->macroSkeleton.append(line + '\n');
        }
      }
      nextLineIsPrototype = true;
    }

    else if (std::regex_search(line, matchMend)) {
      if (this->levelCounter == 0) {
        this->actualParameters.clear();
      } else if (!this->isExpanding) {
        for (auto i = this->formalParameters.begin();
             i != this->formalParameters.end();) {
          if ((*i).coordinates.level == this->levelCounter &&
              !this->formalParameters.empty()) {
            i = this->formalParameters.erase(i);
          } else {
            i++;
          }
        }
        this->levelCounter--;
        this->currentMacroDefinition->macroSkeleton.append(line + '\n');
      }
    } else {
      if (this->isExpanding) {
        std::sregex_iterator it(line.cbegin(), line.cend(),
                                matchSubstitutionPattern);
        std::sregex_iterator end;

        std::string newLine;
        auto lastMatchPosition = line.cbegin();

        for (; it != end; ++it) {
          std::smatch match = *it;

          newLine += match.prefix().str();
          ParemeterCoordinates coords = ReadSubstitutionPattern(match.str());

          if (coords.level == 1) {
            if (coords.position > 0 &&
                (size_t)coords.position <= this->actualParameters.size()) {
              newLine += this->actualParameters[coords.position - 1];
            } else {
              exit(-1);
            }
          } else {
            newLine +=
                CreateSubstitutionPattern(coords.level - 1, coords.position);
          }
          lastMatchPosition = match.suffix().first;
        }
        newLine.append(lastMatchPosition, line.cend());
        line = newLine;

      } else if (this->levelCounter > 0) {
        std::sregex_iterator it(line.cbegin(), line.cend(),
                                matchFormalParameter);
        std::sregex_iterator end;

        std::string newLine;
        auto lastMatchPosition = line.cbegin();

        for (; it != end; ++it) {
          std::smatch match = *it;
          newLine += match.prefix().str();
          bool foundParameter = false;

          for (auto it = formalParameters.rbegin();
               it != formalParameters.rend(); ++it) {
            if (it->name == match.str()) {
              foundParameter = true;
              newLine += CreateSubstitutionPattern(it->coordinates.level,
                                                   it->coordinates.position);
              break;
            }
          }
          if (!foundParameter) {
            exit(-1);
          }

          lastMatchPosition = match.suffix().first;
        }
        newLine.append(lastMatchPosition, line.cend());
        line = newLine;
      }
      if (this->levelCounter == 0) {
        std::cout << line << std::endl;
      } else {
        this->currentMacroDefinition->macroSkeleton.append(line + '\n');
      }
    }
    if (this->isExpanding) {
      if (!std::getline(expansionStream, line)) {
        this->isExpanding = false;
        std::getline(file, line);
      }
    } else {
      std::getline(file, line);
    }
  }
}

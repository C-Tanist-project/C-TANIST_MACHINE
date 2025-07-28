#include "preprocessor.hpp"

MacroProcessor::MacroProcessor(const std::string &asmFilePath) {
  this->isExpanding = false;
  this->asmFilePath = asmFilePath;
  this->levelCounter = 0;
}

void MacroProcessor::Pass() {
  std::ifstream file(asmFilePath);
  std::string line;
  std::regex matchMacro("\\bmacro\\b", std::regex_constants::icase);
  std::regex matchMend("\\bmend\\b", std::regex_constants::icase);
  std::regex matchParameter("&\\w+\\b", std::regex_constants::icase);
  Macro_t *currentMacroDefinition = NULL;
  Macro_t *currentMacroCall = NULL;
  bool nextLineIsPrototype = false;

  if (!file.is_open()) {
    perror("Não foi possível abrir o arquivo.");
    exit(-1);
  }

  std::getline(file, line);
  while (!line.empty()) {
    if (nextLineIsPrototype && currentMacroDefinition != NULL &&
        !this->isExpanding) {
      int positionCounter = 0;
      MacroParameter_t newParameter;

      std::stringstream lineStream(line);
      std::string token;
      lineStream >> token;
      while (lineStream >> token) {
        std::stringstream subTokenString(token);
        std::string subtoken;
        while (std::getline(subTokenString, subtoken, ',')) {
          if (std::regex_match(subtoken, matchParameter)) {
            newParameter.level = levelCounter;
            newParameter.position = positionCounter;
            newParameter.parameter.assign(subtoken.erase(0, 1));
            positionCounter++;
            this->formmalParameters.push_back(newParameter);
          } else {
            if (currentMacroDefinition->macroName.empty()) {
              currentMacroDefinition->macroName = subtoken;
            }
          }
        }
      }
      if (levelCounter > 0) {
        currentMacroDefinition->macroSkeleton.append(line);
      }
    }

    if (std::regex_match(line, matchMacro)) {
      this->levelCounter++;
      if (levelCounter == 1) {
        Macro_t newMacro;
        currentMacroDefinition = &newMacro;
        this->definedMacros.push_back(newMacro);
      } else {
        currentMacroDefinition->macroSkeleton.append(line);
      }
    }
  }
}

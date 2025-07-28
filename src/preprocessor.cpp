#include "preprocessor.hpp"

MacroProcessor::MacroProcessor(const std::string &asmFilePath) {
  this->isExpanding = false;
  this->asmFilePath = asmFilePath;
  this->levelCounter = 0;
}

void MacroProcessor::Pass() {}

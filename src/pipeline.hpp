#pragma once
#include "assembler.hpp"
#include "linker.hpp"
#include "preprocessor.hpp"

#include <string>
#include <vector>

class AssemblyPipeline {
  bool isPipelineInitialized;

  std::vector<std::string> rawSourceFiles;
  std::vector<std::string> expandedSourceFiles;
  std::vector<std::string> objectFiles;
  std::vector<std::string> listingFiles;

  MacroProcessor macroProcessor;
  Assembler assembler();
  Linker linker();
  // Loader loader();

public:
  AssemblyPipeline(const std::string &projectName);
  bool AddRawSource(const std::string &filePath);
};

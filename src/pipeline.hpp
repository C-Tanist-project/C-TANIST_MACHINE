#pragma once
#include "assembler.hpp"
#include "linker.hpp"
#include "preprocessor.hpp"

#include <filesystem>
#include <string>
#include <vector>

class AssemblyPipeline {
  bool isPipelineInitialized;

  std::vector<std::string> rawSourceFiles;
  std::filesystem::path projectPath;

  MacroProcessor macroProcessor;
  Assembler assembler;
  Linker linker;
  // Loader loader();

public:
  AssemblyPipeline(const std::string &projectName);
  bool AddRawSource(const std::string &filePath);
  void Pass(void);
};

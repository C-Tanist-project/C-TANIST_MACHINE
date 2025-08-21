#pragma once
#include "assembler.hpp"
#include "linker.hpp"
#include "pipeline.hpp"
#include "preprocessor.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

class AssemblyPipeline {
  bool InitizlizeAssemblyPipeline(std::string projectName);
  bool isPipelineInitialized;

  std::vector<std::string> rawSourceFiles;
  std::vector<std::string> expandedSourceFiles;
  std::vector<std::string> objectFiles;
  std::vector<std::string> listingFiles;

  Assembler assembler();
  Linker linker();
  // Loader loader();

public:
  AssemblyPipeline(const std::string &projectName);
  bool AddRawSource(const std::string &filePath);
};

#include "pipeline.hpp"
#include <filesystem>

void AssemblyPipeline::Pass(void) {
  assembler.Pass(this->projectPath);
  linker.Pass(this->projectPath);
  loader.Pass(this->projectPath);
}

bool AssemblyPipeline::AddRawSource(const std::string &filePath) {
  this->rawSourceFiles.push_back(filePath);
  this->macroProcessor.Pass(filePath);

  return true;
}

AssemblyPipeline::AssemblyPipeline(const std::string &projectName)
    : macroProcessor((std::filesystem::path(projectName) / "ASM").string()) {
  std::filesystem::path projectDir(projectName);
  this->projectPath = projectDir;
  std::vector<std::filesystem::path> subdirs = {
      projectDir / "ASM", projectDir / "OBJ", projectDir / "HPX",
      projectDir / "LST"};

  try {
    if (!std::filesystem::exists(projectDir)) {
      std::filesystem::create_directory(projectDir);
    } else if (!std::filesystem::is_directory(projectDir)) {
      std::cerr << "ERRO: arquivo já existe com o nome do projeto '"
                << projectDir.string() << "'." << std::endl;
      throw(-1);
    }

    for (const auto &path : subdirs) {
      if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
      } else if (!std::filesystem::is_directory(path)) {
        std::cerr << "ERRO: arquivo já existe com o nome do arquivo  '"
                  << path.string() << "'." << std::endl;
        throw(-1);
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Filesystem error: " << e.what() << std::endl;
    throw;
  }

  assembler = *new Assembler();
  linker = *new Linker();
  loader = *new Loader();
}

#include "pipeline.hpp"

AssemblyPipeline::AssemblyPipeline(const std::string &projectName) {
  this->InitizlizeAssemblyPipeline(projectName);
}

bool AssemblyPipeline::AddRawSource(const std::string &filePath) {
  this->rawSourceFiles.push_back(filePath);
  MacroProcessor mp(filePath);
}

bool AssemblyPipeline::InitizlizeAssemblyPipeline(std::string projectName) {
  std::filesystem::path projectDir(projectName);
  std::vector<std::string> subdirs = {"ASM", "OBJ", "HPX"};
  try {
    if (std::filesystem::create_directory(projectDir)) {
    } else {
      if (!std::filesystem::is_directory(projectDir)) {
        std::cerr
            << "Erro: Já existe um arquivo (não um diretório) com o nome '"
            << projectDir.string() << "'." << std::endl;
        return false;
      }
    }

    for (const auto &subdirName : subdirs) {
      std::filesystem::path path = projectDir / subdirName;

      if (std::filesystem::create_directory(path)) {
      } else {
        if (!std::filesystem::is_directory(path)) {
          std::cerr
              << "Erro: Já existe um arquivo (não um diretório) com o nome '"
              << path.string() << "'." << std::endl;
          return false;
        }
      }
    }

  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Ocorreu um erro de filesystem: " << e.what() << std::endl;
    std::cerr << "Código do erro: " << e.code() << std::endl;
    return false;
  }

  return true;
}

#pragma once
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "types.hpp"

class Loader {
 private:
  struct LoadedModule {
    std::string name;
    std::vector<int16_t> code;
    std::unordered_map<int16_t, OperandFormat> relocationTable;
    int16_t stackSize = 0;
    int16_t entryPoint = 0;
    int16_t loadAddress = 0;
  };

  LoadedModule module;  // Módulo carregado

  int16_t currentLoadAddress = 0;
  std::string fileName;

  // Lê HPX e retorna módulo carregado
  LoadedModule ReadHPX();
  // Aplica relocação absoluta ou relativa no módulo
  void RelocateModule();

 public:
  Loader(const std::string &fileName);
  void Pass();
};

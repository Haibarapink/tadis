#pragma once

#include <iostream>


static bool quit = false;

void do_quit() {
  quit = true;
}

void read_file_and_print(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }
  std::string line;
  while (std::getline(file, line)) {
    std::cout << line << std::endl;
  }
  file.close();
}

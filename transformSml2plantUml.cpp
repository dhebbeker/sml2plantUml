#include "sml2plantUml.hpp"
#include <cstdlib>
#include <iostream>

#include HEADER_TO_CHECK

int main() {
  dump<STATE_MACHINE_NAME>(std::cout);
  return EXIT_SUCCESS;
}

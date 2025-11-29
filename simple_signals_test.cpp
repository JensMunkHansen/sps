#include "simple_signals.hpp"
#include <iostream>

int main()
{

  // create new signal
  Signal<std::string, int> signal;

  // attach a slot
  signal.connect([](std::string arg1, int arg2) { std::cout << arg1 << " " << arg2 << std::endl; });

  signal.emit("The answer:", 42);

  return 0;
}

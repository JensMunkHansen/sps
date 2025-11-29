#include <stdio.h>
#include <string.h>

#ifdef __GNUG__
#include <unistd.h>
#endif

#include <errno.h>
#include <stdlib.h>

#include <chrono>
#include <iostream>

#include <sps/thread.hpp>

#define NAMELEN 16

int main(int argc, char* argv[])
{
  SPS_UNREFERENCED_PARAMETERS(argc, argv);
  char outputName[NAMELEN];

  sps::thread mt1([]() noexcept { std::this_thread::sleep_for(std::chrono::milliseconds(5000)); });

  mt1.set_name("Hello");
  mt1.get_name(outputName, NAMELEN);
  std::cout << outputName << std::endl;

  std::cout << "thread 1 active = " << std::boolalpha << mt1.isActive() << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "thread 1 active = " << std::boolalpha << mt1.isActive() << std::endl;
  mt1.join();
  return 0;
}

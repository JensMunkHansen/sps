#include "demo.h"

using namespace std;

int main()
{
  std::cout << "==== Basic demo ====" << std::endl;
  signals_basic_demo();
  std::cout << std::endl;

  std::cout << "==== Track objects ====" << std::endl;
  signals_track_objects();
  std::cout << std::endl;

  std::cout << "==== Cross thread calls ====" << std::endl;
  signals_cross_thread_calls();
  std::cout << std::endl;

  return 0;
}
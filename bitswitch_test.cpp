#include "bitswitch.hpp"
#include <iostream>

using std::cout;
using std::endl;
using std::cin;

using sps::bool2int;
using sps::operator "" _bin2int;

int main() {
  const size_t kBooleans = 2;
  bool bArray[kBooleans];
  for (size_t i = 0 ; i < kBooleans ; i++) {
    cin >> bArray[i];
  }

  uint32_t mask = bool2int(2, bArray[0], bArray[1]);

  switch (mask) {
  case "00"_bin2int:
    cout << "both false" << endl;
    break;
  case "01"_bin2int:
    cout << "first false, second true" << endl;
    break;
  case "10"_bin2int:
    cout << "first true, second false" << endl;
    break;
  case "11"_bin2int:
    cout << "both true" << endl;
    break;
  default:
    break;
  }

  uint32_t basis = 0;
  cout << basis << endl;
  return 0;
}

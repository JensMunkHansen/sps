#include <sps/pimpl_test.hpp>

#include <cstdio>
#include <cstdlib>

class PimplTestPrivate
{
  SPS_DECLARE_PUBLIC(PimplTest);

protected:
  PimplTest* const q_ptr;

public:
  PimplTestPrivate(PimplTest& object);
  void init();
};

PimplTestPrivate::PimplTestPrivate(PimplTest& object)
  : q_ptr(&object)
{
}

void PimplTestPrivate::init()
{
  SPS_Q(PimplTest);
  q->doNothing();
  printf("init\n");
}

PimplTest::PimplTest()
  : d_ptr(new PimplTestPrivate(*this))
{
  // alternatively
  // this->d_ptr = std::make_unique<PimplTestPrivate>(*this);
  SPS_D(PimplTest);
  d->init();
}

void PimplTest::doNothing()
{
  printf("%s\n", SPS_FUNC_INFO);
}

PimplTest::~PimplTest() = default;

int main(int argc, char* argv[])
{

  PimplTest p;
  return EXIT_SUCCESS;
}

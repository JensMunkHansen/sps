#include <sps/sps_macros.hpp>

#include <memory>

class PimplTestPrivate;

class PimplTest {
 public:
  PimplTest();
  virtual ~PimplTest();
  void doNothing();
 protected:
  std::unique_ptr<PimplTestPrivate> d_ptr;
 private:
  SPS_DECLARE_PRIVATE(PimplTest);
  SPS_DISABLE_COPY(PimplTest);
};

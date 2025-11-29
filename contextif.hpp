/**
 * @file   context.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Tue Feb 27 00:46:20 2018
 *
 * @brief
 *
 *
 */

namespace sps
{
class ContextIF
{
public:
  static int Create(ContextIF** ppContext);
  static int Destroy(ContextIF* pContext);

  virtual ~ContextIF() = default;
  ContextIF(ContextIF&& other) = default;
  ContextIF& operator=(ContextIF&& other) = default;

protected:
  ContextIF() = default;

private:
  ContextIF(const ContextIF& rhs) = delete;
  ContextIF& operator=(const ContextIF& rhs) = delete;
};
} // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */

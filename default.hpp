#pragma once

#include <sps/optional>

namespace sps
{

struct DefaultValue
{
};
static const DefaultValue defaultValue;

template <typename T>
using IsNotReference = typename std::enable_if<!std::is_reference<T>::value, void>::type;

template <typename T, T... DefaultedParameter>
class Defaulted
{
public:
  // Do not make these explicit
  Defaulted(T const& t)
    : value_(t)
  {
  }

  template <typename T_ = T, typename = IsNotReference<T_>>
  Defaulted(T&& t)
    : value_(std::move(t))
  {
  }

  Defaulted(DefaultValue)
    : value_(DefaultedParameter...)
  {
  }
  T const& get_or_default() const { return value_; }
  T& get_or_default() { return value_; }

private:
  T value_;
};

template <typename T, typename GetDefaultValue>
class DefaultedF
{
public:
  DefaultedF(T const& t)
    : value_(t)
  {
  }

  template <typename T_ = T, typename = IsNotReference<T_>>
  DefaultedF(T&& t)
    : value_(std::move(t))
  {
  }

  DefaultedF(DefaultValue)
    : value_(sps::nullopt)
  {
  }

  template <typename... Args>
  T get_or_default(Args&&... args)
  {
    if (value_)
    {
      return *value_;
    }
    else
    {
      return GetDefaultValue::get(std::forward<Args>(args)...);
    }
  }

private:
  sps::optional<T> value_;
};
} // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */

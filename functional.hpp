/**
* @file   functional.hpp
* @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
* @date   Sat Feb 24 17:40:38 2018
*
* @brief
*
*
*/
/*
 *  This file is part of SOFUS.
 *
 *  SOFUS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SOFUS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SOFUS.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#include <functional>
#include <type_traits>
#include <new>

#include "Assert.h"

namespace sps {

template <typename CT, typename ... A> struct function_helper
: public function_helper<decltype(&CT::operator())(A...)> {};

template <typename C> struct function_helper<C> {
 private:
  C mObject;

 public:
  explicit function_helper(const C & obj) : mObject(obj) {}

  template<typename... Args> typename
  std::result_of<C(Args...)>::type operator()(Args... a) {
    return this->mObject.operator()(a...);
  }

  template<typename... Args> typename
  std::result_of<const C(Args...)>::type operator()(Args... a) const {
    return this->mObject.operator()(a...);
  }
};

namespace make {

/**
 * @brief make::function
 *
 * @tparam C type of callable
 * @param obj callable
 *
 * @return Function pointer with the signature of the callable
 */
template<typename C>
auto function(const C & obj) {
  return sps::function_helper<C>(obj);
}
}  // namespace make

/*! \brief static_function
 *
 * @tparam Signature
 * @tparam Size
 *
 * Replacement for std::function, which uses stack allocation. There
 * are 3 issues with std::function: It uses dynamic memory allocation,
 * it throws exceptions in case the function is invoked without
 * assigning callable object to it first and it executes a virtual
 * function call. The reason for the virtual function call is because
 * under the hood, a lambda creates a class that contains the code and
 * all the captured data you specified for capture. Since other
 * callers of your std::function have no idea what the type of that
 * object is (it’s only visible in the scope the lambda was defined
 * in, and is not a human friendly name), the std::function has to use
 * a virtual function to “blindly” call into from the outside, to
 * perform the correct work on the inside. This technique is called
 * “type erasure”.
 *
 * It seems that in most cases the callback object will contain
 * pointer to member function, pointer to handling object and some
 * additional single parameter. This is the reason for specifying the
 * default storage space as equal to the size of 3 pointers. The
 * “signature” template parameter is exactly the same as with
 * std::function plus an optional storage area size template
 * parameter:
 */
template <typename Signature, std::size_t Size = sizeof(void*) * 3>
class static_function;

// Change to Ret, Args, Size
template <std::size_t Size, typename Ret, typename... Args>
class static_function<Ret(Args...), Size> {
 public:
  /// @brief Result type
  typedef Ret result_type;

  static const std::size_t size = Size;

  /// @brief Default constructor
  static_function();

  /// @brief Constructs static_function object out of provided functor
  /// @pre F invocation must have the same signature as static_function
  /// @pre @code sizeof(F) <= Size @endcode
  template <typename Func>
  explicit static_function(Func&& func);

  /// @brief Copy constructor
  static_function(const static_function& other);

  /// @brief Move constructor
  static_function(static_function&& other);

  /// @brief Destructor
  ~static_function();

  /// @brief Copy assignment operator
  static_function& operator=(const static_function& other);

  /// @brief Non-const param copy assignment operator
  static_function& operator=(static_function& other);

  /// @brief Move assignment operator
  /// @post Other function becomes invalid: @code (!other) == true @endcode
  static_function& operator=(static_function&& other);

  /// @brief Invalidates current function.
  /// @post This function becomes invalid: @code (!(*this)) == true @endcode
  static_function& operator=(std::nullptr_t);

  /// @brief Assigns new functor to current function using move semantics.
  /// @pre F invocation must have the same signature as static_function
  /// @pre @code sizeof(F) <= Size @endcode
  /// @post This function becomes valid: @code (!(*this)) == false @endcode
  template <typename Func>
  static_function& operator=(Func&& func);

  /// @brief Assigns new functor to current function using copy semantics.
  /// @pre F invocation must have the same signature as static_function
  /// @pre @code sizeof(F) <= Size @endcode
  /// @post This function becomes valid: @code (!(*this)) == false @endcode
  template <typename Func>
  static_function& operator=(std::reference_wrapper<Func> func);

  /// @brief Boolean conversion operator.
  /// @return Returns true if and only if current function is valid, i.e.
  ///         may be invoked using operator().
  operator bool() const;

  /// @brief Negation operator.
  /// @return Returns true if and only if current function is invalid, i.e.
  ///         may NOT be invoked using operator().
  bool operator!() const;

  /// @brief Function invocation operator.
  /// @details Invokes operator() of the stored functor with provided arguments
  /// @return What functor returns
  /// @pre The function object is valid, i.e. has functor assigned to it.
  Ret operator()(Args... args) const;

  /// @brief Non-const version of operator().
  Ret operator()(Args... args);

 private:
  /// @cond DOCUMENT_STATIC_FUNCTION_INVOKER
  class Invoker {
   public:
    virtual ~Invoker();
    virtual Ret exec(Args... args) const = 0;
    virtual Ret exec(Args... args) = 0;
    virtual void copyTo(void* other) const = 0;
    virtual void moveTo(void* other) = 0;
   private:
  };

  template <typename TBound>
  class InvokerBound : public Invoker {
   public:
    template <typename Func>
    explicit InvokerBound(Func&& func);
    InvokerBound(const InvokerBound&) = default;
    InvokerBound(InvokerBound&&) = default;
    virtual ~InvokerBound();
    virtual Ret exec(Args... args) const;
    virtual Ret exec(Args... args);
    virtual void copyTo(void* other) const;
    virtual void moveTo(void* other);
   private:
    TBound func_;
  };
  /// @endcond

  static const std::size_t StorageAreaSize = Size + sizeof(Invoker);
  typedef typename
  std::aligned_storage<StorageAreaSize,
      std::alignment_of<Invoker>::value>::type StorageType;

  Invoker* getInvoker();
  const Invoker* getInvoker() const;
  void destroyHandler();
  template <typename Func>
  void assignHandler(Func&& func);

  StorageType handler_;
  bool valid_;
};

/// @}

// Implementation
template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>::static_function() : valid_(false) {}

template <std::size_t Size, typename Ret, typename... Args>
template <typename Func>
static_function<Ret(Args...), Size>::static_function(Func&& func)
  : valid_(true) {
  assignHandler(std::forward<Func>(func));
}

template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>::static_function(
  const static_function& other)
  : valid_(other.valid_) {
  if (valid_) {
    auto otherInvoker = other.getInvoker();
    otherInvoker->copyTo(&handler_);
  }
}

template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>::static_function(
  static_function&& other)
  : valid_(other.valid_) {
  if (valid_) {
    auto otherInvoker = other.getInvoker();
    otherInvoker->moveTo(&handler_);
    other = nullptr;
  }
}

template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>::~static_function() {
  destroyHandler();
}

template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>&
static_function<Ret(Args...), Size>::operator=(const static_function& other) {
  if (&other == this) {
    return *this;
  }

  destroyHandler();
  valid_ = other.valid_;
  if (valid_) {
    auto otherInvoker = other.getInvoker();
    //GASSERT(otherInvoker != nullptr);
    otherInvoker->copyTo(&handler_);
  }
  return *this;
}

template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>&
static_function<Ret(Args...), Size>::operator=(static_function& other) {
  return operator=(static_cast<const static_function&>(other));
}

template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>&
static_function<Ret(Args...), Size>::operator=(static_function&& other) {
  if (&other == this) {
    return *this;
  }

  destroyHandler();
  valid_ = other.valid_;
  if (valid_) {
    auto otherInvoker = other.getInvoker();
    //GASSERT(otherInvoker != nullptr);
    otherInvoker->moveTo(&handler_);
    other = nullptr;
  }
  return *this;
}

template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>&
static_function<Ret(Args...), Size>::operator=(std::nullptr_t) {
  destroyHandler();
  valid_ = false;
  return *this;
}

template <std::size_t Size, typename Ret, typename... Args>
template <typename Func>
static_function<Ret (Args...), Size>&
static_function<Ret (Args...), Size>::operator=(Func&& func) {
  destroyHandler();
  assignHandler(std::forward<Func>(func));
  valid_ = true;
  return *this;
}

template <std::size_t Size, typename Ret, typename... Args>
template <typename Func>
static_function<Ret (Args...), Size>&
static_function<Ret (Args...), Size>::operator=(
  std::reference_wrapper<Func> func) {
  destroyHandler();
  assignHandler(std::forward<Func>(func));
  valid_ = true;
  return *this;
}

template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret (Args...), Size>::operator bool() const {
  return valid_;
}

template <std::size_t Size, typename Ret, typename... Args>
bool static_function<Ret (Args...), Size>::operator!() const {
  return !valid_;
}

template <std::size_t Size, typename Ret, typename... Args>
Ret static_function<Ret (Args...), Size>::operator()(
  Args... args) const {
  //GASSERT(valid_);
  auto invoker = getInvoker();
  return invoker->exec(std::forward<Args>(args)...);
}

template <std::size_t Size, typename Ret, typename... Args>
Ret static_function<Ret (Args...), Size>::operator()(
  Args... args) {
  //GASSERT(valid_);
  auto invoker = getInvoker();
  return invoker->exec(std::forward<Args>(args)...);
}

/// @cond DOCUMENT_STATIC_FUNCTION_INVOKER
template <std::size_t Size, typename Ret, typename... Args>
static_function<Ret(Args...), Size>::Invoker::~Invoker() {}

template <std::size_t Size, typename Ret, typename... Args>
template <typename TBound>
template <typename Func>
static_function<Ret(Args...), Size>::InvokerBound<TBound>::InvokerBound(
  Func&& func)
  : func_(std::forward<Func>(func)) {}

template <std::size_t Size, typename Ret, typename... Args>
template <typename TBound>
static_function<Ret(Args...), Size>::InvokerBound<TBound>::~InvokerBound() {}

template <std::size_t Size, typename Ret, typename... Args>
template <typename TBound>
Ret static_function<Ret(Args...), Size>::InvokerBound<TBound>::exec(
  Args... args) const {
  return func_(std::forward<Args>(args)...);
}

template <std::size_t Size, typename Ret, typename... Args>
template <typename TBound>
Ret static_function<Ret(Args...), Size>::InvokerBound<TBound>::exec(
  Args... args) {
  return func_(std::forward<Args>(args)...);
}

template <std::size_t Size, typename Ret, typename... Args>
template <typename TBound>
void static_function<Ret(Args...), Size>::InvokerBound<TBound>::copyTo(
  void* place) const {
  auto otherInvoker = new (place) InvokerBound(*this);
  static_cast<void>(otherInvoker);
}

template <std::size_t Size, typename Ret, typename... Args>
template <typename TBound>
void static_function<Ret(Args...), Size>::InvokerBound<TBound>::moveTo(
  void* place) {
  auto otherInvoker = new (place) InvokerBound(std::move(*this));
  static_cast<void>(otherInvoker);
}
/// @endcond

template <std::size_t Size, typename Ret, typename... Args>
typename static_function<Ret(Args...), Size>::Invoker*
static_function<Ret(Args...), Size>::getInvoker() {
  return reinterpret_cast<Invoker*>(&handler_);
}

template <std::size_t Size, typename Ret, typename... Args>
const typename static_function<Ret(Args...), Size>::Invoker*
static_function<Ret(Args...), Size>::getInvoker() const {
  return reinterpret_cast<const Invoker*>(&handler_);
}

template <std::size_t Size, typename Ret, typename... Args>
void static_function<Ret(Args...), Size>::destroyHandler() {
  if (valid_) {
    auto invoker = getInvoker();
    invoker->~Invoker();
  }
}

template <std::size_t Size, typename Ret, typename... Args>
template <typename Func>
void static_function<Ret(Args...), Size>::assignHandler(Func&& func) {
  typedef static_function<Ret(Args...), Size> ThisType;
  typedef typename std::decay<Func>::type DecayedFuncType;
  typedef InvokerBound<DecayedFuncType> InvokerBoundType;

  static_assert(!std::is_same<ThisType, DecayedFuncType>::value,
                "Wrong function invocation");

  static_assert(sizeof(InvokerBoundType) <= StorageAreaSize,
                "Increase the Size template argument of the static_function");

  static_assert(alignof(Invoker) == alignof(InvokerBoundType),
                "Alignment requirement for Invoker object must be the same as "
                "alignment requirement for InvokerBoundType type object");

  auto handlerPtr = new (&handler_) InvokerBoundType(std::forward<Func>(func));
  static_cast<void>(handlerPtr);
}
}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */

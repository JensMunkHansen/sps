/**
 * @file   weak_binder.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Wed Apr 25 23:47:37 2018
 *
 * @brief Weak binding for callables
 *
 * The construct can be used for dispatching callbacks, when you are
 * not guaranteed that a target object still exists when an
 * asynchronous handler is invoked.
 *
 */
#if defined(__GNUG__) && (__GNUG__ < 7)
# include <experimental/optional>
#else
# include <optional>
# include <functional>
#endif

#include <memory>

// #include <boost/none.hpp>

namespace sps {

/*
  // void_t trick
  template<typename... Ts> struct make_void { typedef void type;};
  template<typename... Ts> using void_t = typename make_void<Ts...>::type;
 */

struct called_flag {};

#if defined(__GNUG__) && (__GNUG__ < 7)
  using std::experimental::optional;
  using std::experimental::nullopt;
#else
  using std::optional;
  using std::nullopt;
#endif

namespace detail {
template <class Target, class Fun>
struct weak_binder {
  using target_type = Target;
  using weak_ptr_type = std::weak_ptr<Target>;

  weak_binder(weak_ptr_type weak_ptr, Fun fun) :
    m_weak_ptr(std::move(weak_ptr)), m_fun(std::move(fun)) {}

  template <class... Args, class Result = std::result_of_t<Fun(Args...)>,
            std::enable_if_t<not std::is_void<Result>::value>* = nullptr>
  auto operator()(Args&& ...args) const -> optional<Result> {
    auto locked_ptr = m_weak_ptr.lock();
    if (locked_ptr) {
      return m_fun(std::forward<Args>(args)...);
    } else {
      return nullopt;
    }
  }

  template<class...Args,
           class Result = std::result_of_t<Fun(Args...)>,
           std::enable_if_t<std::is_void<Result>::value>* = nullptr>
  auto operator()(Args&&...args) const -> optional<called_flag> {
    auto locked_ptr = m_weak_ptr.lock();
    if (locked_ptr) {
      m_fun(std::forward<Args>(args)...);
      return called_flag {};
    } else {
      return nullopt;
    }
  }

  weak_ptr_type m_weak_ptr;
  Fun m_fun;
};
}  // namespace detail

/**
 * Versions for handling member functions
 *
 *
 * @return
 */
template<class Ret, class Target, class...FuncArgs, class...Args>
auto bind_weak(Ret (Target::*mfp)(FuncArgs...),
               const std::shared_ptr<Target>& ptr, Args&&...args) {
  using binder_type =
    decltype(std::bind(mfp, ptr.get(), std::forward<Args>(args)...));
  return detail::weak_binder<Target, binder_type> {
    std::weak_ptr<Target>(ptr),
    std::bind(mfp, ptr.get(), std::forward<Args>(args)...)
  };
}

//

/**
 * version for calling any callable when an unrelated weak_ptr can be locked
 *
 * @param f
 * @param ptr
 * @param args
 *
 * @return
 */
template<class Callable, class Pointee, class...Args>
auto bind_weak(Callable&& f,
               const std::shared_ptr<Pointee>& ptr, Args&&...args) {
  using binder_type =
    decltype(std::bind(std::forward<Callable>(f),
                       std::forward<Args>(args)...));
  return detail::weak_binder<Pointee, binder_type> {
    std::weak_ptr<Pointee>(ptr),
    std::bind(std::forward<Callable>(f), std::forward<Args>(args)...)
  };
}

/**
 * Optimisation for no arguments
 *
 * @param f
 * @param ptr
 *
 * @return
 */
template<class Callable, class Pointee>
auto bind_weak(Callable&& f, const std::shared_ptr<Pointee>& ptr) {
  return detail::weak_binder<Pointee, Callable> {
    std::weak_ptr<Pointee>(ptr),
    std::forward<Callable>(f)
  };
}
}  // namespace sps

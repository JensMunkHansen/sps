#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

void f(int& i)
{
  ++i;
}

void f(std::string& s);

// C++ requires you to type out the same function body three times to
// obtain SFINAE-friendliness and noexcept-correctness. That's
// unacceptable.
#define RETURNS(...)                                                                               \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__)                                           \
  {                                                                                                \
    return __VA_ARGS__;                                                                            \
  }

// Second, this solution maintains the noexcept quality of each
// overload of f, be it true or false:

// This way, the wrapper around f doesn’t add a specific behaviour. It
// behaves very much like it was just f we called, except that it
// takes care of the overloading resolution. Which was exactly the
// purpose of the challenge.

// Finally, using decltype(__VA_ARGS__) instead of just decltype(auto)
// helps compile-time evaluations like std::is_invocable figure out
// the type of what the function could be returning, depending on its
// arguments. Indeed such contexts don’t instantiate the body of the
// template function to determine what decltype(auto) resolves
// to. This is useful in SFINAE contexts for example.

// The name of overload sets can be legally used as part of a function
// call - we can use a macro to create a lambda for us that "lifts"
// the overload set into a function object.
#define LIFT(f) [](auto&&... xs) RETURNS(f(::std::forward<decltype(xs)>(xs)...))

/*  Double functor trick */

class Employee
{
public:
  explicit Employee(int i)
    : id_(i)
  {
  }
  int getId() const { return id_; }
  bool operator<(const Employee& other) const { return this->id_ < other.getId(); }

private:
  int id_;
};

struct CompareWithId
{
  bool operator()(Employee const& employee, int id) { return employee.getId() < id; }
  bool operator()(int id, Employee const& employee) { return id < employee.getId(); }
};

template <typename B1, typename B2>
struct Merged
  : B1
  , B2
{
  Merged(B1 b1, B2 b2)
    : B1(std::move(b1))
    , B2(std::move(b2))
  {
  }

  // Needed here
  using B1::operator();
  using B2::operator();
};

#if 0
template <typename ... B>
struct Merged2 : B... {
  template <typename ... T>
  Merged2(T && ... t) : B(std::forward<T>(t))...
  {

  }

  using B::operator()...;
};

// C++17 does not allow constructor to have different parameter packs than struct
template<typename ... >
Merged2(T...) -> Merged2<std::decay_t<T>...>;
#endif

int main()
{
  const auto l1 = []() { return 4; };
  const auto l2 = [](const int i) { return i * 10; };

  Merged<decltype(l1), decltype(l2)> merged(l1, l2);

  std::cout << merged(2) << std::endl;

#if 0
  Merged2 merged2(l1, l2, [](const double d) { return d*3.2; },
                  [i = std::make_unique<int>(5)](char c){} );
#endif

  std::set<Employee> employees;
  employees.insert(Employee(2));
  employees.insert(Employee(43));
  employees.insert(Employee(27));

  std::set<int> ids;
  ids.insert(2);
  ids.insert(43);
  ids.insert(27);
  ids.insert(14);

  std::vector<int> idsToClean = std::vector<int>();

  std::set_difference(ids.begin(), ids.end(), employees.begin(), employees.end(),
    std::back_inserter(idsToClean), CompareWithId());

  std::cout << idsToClean.size() << std::endl;

  std::vector<int> numbers = { 1, 2, 3, 4, 5 };
  //  std::for_each(begin(numbers), end(numbers), f);
  std::for_each(begin(numbers), end(numbers), LIFT(f));
  return 0;
}

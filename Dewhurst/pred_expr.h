//
// Copyright (c) 2017 by Daniel Saks and Stephen C. Dewhurst.
//
// Permission to use, copy, modify, distribute, and sell this
// software for any purpose is hereby granted without fee, provided
// that the above copyright notice appears in all copies and
// that both that copyright notice and this permission notice
// appear in supporting documentation.
// The authors make no representation about the suitability of this
// software for any purpose. It is provided "as is" without express
// or implied warranty.
//

#ifndef INCLUDED_PRED_EXPR
#define INCLUDED_PRED_EXPR

#include <cstddef>
#include <type_traits>

//
// Facility to create a compile-time abstract syntax tree (AST) that
// represents a compound predicate composed of class template predicates.
// Each predicate takes a typename argument and produce a compile time
// boolean result when specialized.
//
// Can be used to make "poor man's concepts" less impoverished.
//
namespace DandS
{
inline namespace ASTImplementation
{
//
// Convenience template for generating predicates.
//
template <bool c>
using truthiness = std::conditional_t<c, std::true_type, std::false_type>;

//
// Quick check to see if a series of zero or more bools all evaluate to true.
//
template <bool... bs>
struct AllTrue;

template <bool b, bool... bs>
struct AllTrue<b, bs...>
{
  enum : bool
  {
    value = b && AllTrue<bs...>::value
  };
};

template <>
struct AllTrue<>
{
  enum : bool
  {
    value = true
  };
};

//
// See if each Ts is derived from Base.
//
template <typename Base, typename... Ts>
struct all_derived_from;

template <typename Base, typename F, typename... Rs>
struct all_derived_from<Base, F, Rs...>
{
  enum : bool
  {
    value = std::is_base_of<Base, F>::value && all_derived_from<Base, Rs...>::value
  };
};

template <typename Base>
struct all_derived_from<Base>
{
  enum : bool
  {
    value = true
  };
};

//
// Every AST node is-a "Predicate AST"
//
struct PAST
{
};

//
// Check that all are derived from PAST.
//
template <typename... Ts>
using all_PAST = all_derived_from<PAST, Ts...>;
}

//
// The language consists of type predicate leaves
// the binary operators &, |, and ^ (and, or, and exclusive or),
// and ! (negation).
//
// We use the compiler's parser to parse the expressions so (of course)
// the precedence and associativity match those of C++.
//
// We leverage the compiler's symbol table to build our compile-time
// ASTs and evaluate them with compile-time template arguments.
//
// This facility produces no runtime executable code.

//====================
// AST leaf nodes:
//     a) ids that wrap a predicate template that can be specilaized with one typename
//     b) binders that bind an argument of a binary predicate to produce an id

// An identifier type that wraps a predicate template.  The template
// takes a typename and passes judgement on it at compile time.
//
// Note use of variadic template template to allow predicates that take trailing defaults.
//
template <template <typename...> class Pred>
struct Id : PAST
{
  template <typename T>
  static constexpr bool eval()
  {
    return Pred<T>::value;
  }
};
template <template <typename...> class Pred>
constexpr auto pred()
{
  return Id<Pred>();
}

// Binders that take a binary template and either a type
// or a size_t argument and produce something that can be
// used as an Id.
//
// Bind the first argument of a binary predicate that takes [typename, typename].
//
template <template <typename, typename> class BPred, typename First>
struct BindT1st : PAST
{
  template <typename Second>
  static constexpr bool eval()
  {
    return BPred<First, Second>::value;
  }
};
template <template <typename, typename> class BPred, typename First>
constexpr auto bind1st()
{
  return BindT1st<BPred, First>();
}

// Bind the second argument of a of a binary predicate that takes [typename, typename].
//
template <template <typename, typename> class BPred, typename Second>
struct BindT2nd : PAST
{
  template <typename First>
  static constexpr bool eval()
  {
    return BPred<First, Second>::value;
  }
};
template <template <typename, typename> class BPred, typename Second>
constexpr auto bind2nd()
{
  return BindT2nd<BPred, Second>();
}

// Bind the first argument of a binary predicate that takes [size_t, typename].
//
template <template <size_t, typename> class BPred, size_t first>
struct BindI1st : PAST
{
  template <typename Second>
  static constexpr bool eval()
  {
    return BPred<first, Second>::value;
  }
};
template <template <size_t, typename> class BPred, size_t first>
constexpr auto bind1st()
{
  return BindI1st<BPred, first>();
}

// Bind the second argument of a of a binary predicate that takes [typename, size_t].
//
template <template <typename, size_t> class BPred, size_t second>
struct BindI2nd : PAST
{
  template <typename First>
  static constexpr bool eval()
  {
    return BPred<First, second>::value;
  }
};
template <template <typename, size_t> class BPred, size_t second>
constexpr auto bind2nd()
{
  return BindI2nd<BPred, second>();
}

// Operators used to build the compile-time AST.
// We overload the bitwise operators &, |, and ^ rather than
// && and || so that we can have an ^ operation and also so as
// not to imply that the operators will short-circuit.
// We use ! for negation rather than ~, however.
//
template <typename P1, typename P2>
struct And : PAST
{
  template <typename T>
  static constexpr bool eval()
  {
    return P1::template eval<T>() & P2::template eval<T>();
  }
};
template <typename P1, typename P2, typename = all_PAST<P1, P2>>
constexpr auto operator&(P1, P2)
{
  return And<P1, P2>();
}

template <typename P1, typename P2>
struct Or : PAST
{
  template <typename T>
  static constexpr bool eval()
  {
    return P1::template eval<T>() | P2::template eval<T>();
  }
};
template <typename P1, typename P2, typename = all_PAST<P1, P2>>
constexpr auto operator|(P1, P2)
{
  return Or<P1, P2>();
}

template <typename P1, typename P2>
struct Xor : PAST
{
  template <typename T>
  static constexpr bool eval()
  {
    return P1::template eval<T>() ^ P2::template eval<T>();
  }
};
template <typename P1, typename P2, typename = all_PAST<P1, P2>>
constexpr auto operator^(P1, P2)
{
  return Xor<P1, P2>();
}

template <typename P>
struct Not : PAST
{
  template <typename T>
  static constexpr bool eval()
  {
    return !P::template eval<T>();
  }
};
template <typename P, typename = all_PAST<P>>
constexpr auto operator!(P)
{
  return Not<P>();
}

// Another way to evaluate an AST object.  In addition to
//     ast.eval<T>()
// this provides the syntax
//     satisfies<T>(ast)
//
// Can also check multiple arguments against the constraint:
//    satisfies<T1, T2, T3>(ast)
//
template <typename... Ts, typename AST, typename = all_PAST<AST>>
constexpr bool satisfies(AST)
{
  return AllTrue<AST().template eval<Ts>()...>::value;
}

// For convenience with SFINAE applications.
// Uses an AST type:
//    Constraint<AST, T>
// checks whether T satisfies the constraints described by AST.
// The result is either void or represents no type.
//
// Can test multiple arguments against the constraint:
//    Constraint<AST, T1, T2, T3>
//
template <typename AST, typename... Ts>
using Constraint = typename std::enable_if<satisfies<Ts...>(AST())>::type;

// A bool SFINAE constraint.
// Uses an AST object rather than a type.  Same syntax as "satisfies,"
// but result is either true or produces no result.
// Implementation of constraint inspired by example in
// http://en.cppreference.com/w/cpp/language/sfinae
//
template <typename... Ts, typename AST, bool test = AllTrue<AST().template eval<Ts>()...>::value>
constexpr bool constraint(AST, char (*)[test == true] = nullptr)
{
  return true;
}

//
// Handy but optional names to use as "trailing distinguishers" for
// overloaded member templates that are otherwise identical.
//
template <size_t>
struct Ignore
{
};
constexpr bool ignore(size_t)
{
  return true;
}
}

#endif

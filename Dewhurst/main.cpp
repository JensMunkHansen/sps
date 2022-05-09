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

#include "pred_expr.h"
#include "pred_expr_traits.h"
#include "stl_iterator_traits.h"

#include "shape.h"

#include <iostream>
#include <exception>
#include <type_traits>
#include <list>
#include <vector>
#include <typeinfo>

namespace DandS {
	//
	// Some handcrafted predicates.
	//

	//
	// Here's a binary predicate; are A and B "similar" types?
	//
	template <typename A, typename B>
	using are_similar = std::is_same<typename std::decay<A>::type, typename std::decay<B>::type>;

	//
	// Here are binary predicates about sizes:
	//
	template <typename T, size_t n>
	struct is_smaller_than : truthiness<(sizeof(T) < n)> {};

	template <typename T, size_t n>
	struct is_bigger_than : truthiness<(sizeof(T) > n)> {};

	template <typename T, size_t n>
	struct is_of_size : truthiness<(sizeof(T) == n)> {};

	// As with STL comparators, sometimes you need an object and
	// sometimes you need a type.  For example, set<T,Comp> requires a
	// type, and sort(b, e, Comp()) requires an object.
	// STL has at least some runtime component, so the type eventually
	// becomes an object when used.
	//
	// Here, we're doing pure metaprogramming, so our object eventually
	// is used only as a type.

	//
	// Here's a binder that produces a unary predicate; is this type similar to Shape?
	//
	constexpr auto isSimilarToShape = bind2nd<are_similar, Shape>;

	// Here are binders that bind a numeric argument;
	// is this type small, is this type the size of a pointer to data?
	constexpr auto isSmall = bind2nd<is_smaller_than, 8>();
	constexpr auto isPtrSize = bind2nd<is_of_size, sizeof(void *)>();

	//
	// Here's an AST object (that has no runtime component);
	// is this type an interface class?
	//
	constexpr auto isInterfaceClass =
		isPtrSize &
		hasVirtualDestructor &
		isAbstract;

	//
	// Here's the type of the AST object.
	// An AST object and its type carry the same information
	// but are used differently.
	//
	using IsInterfaceClass = decltype(isInterfaceClass);

	//
	// Here's a class template that has a constraint on the type used for specialization.
	// We're using Constraint with the type of the AST.
	//
	template <typename T, typename = Constraint<IsInterfaceClass, T>>
	class InterfaceOnly1 {
		// implementation...
	};
	InterfaceOnly1<Shape> interface_only1;

	//
	// Here's a class template that has a constraint on the type used for specialization.
	// Here we're using constraint with an AST object.
	//
	template <typename T, bool = constraint<T>(isInterfaceClass)>
	class InterfaceOnly1_1 {
		// implementation...
	};
	InterfaceOnly1_1<Shape> interface_only1_1;

	//
	// Here we'll use satisfies with an AST object in a static assertion.
	//
	template <typename T>
	class InterfaceOnly2 {
		static_assert(satisfies<T>(isInterfaceClass), "template argument must be an interface class.");
		static_assert(satisfies<T>(IsInterfaceClass()), "template argument must be an interface class.");
		// implementation...
	};
	InterfaceOnly2<Shape> interface_only2;

	//
	// Here we'll check multiple arguments:
	//
	template <typename T1, typename T2, typename = Constraint<IsInterfaceClass, T1, T2>>
	class InterfacesOnly1 {
		// implementation...
	};
	InterfacesOnly1<Shape, Shape> interfaces_only1;

	//
	// Here we'll use it in a static assertion:
	//
	template <typename T1, typename T2, typename T3>
	class InterfacesOnly2 {
		static_assert(satisfies<T1, T2, T3>(isInterfaceClass), "template arguments must be interface classes.");
		// implementation...
	};
	InterfacesOnly2<Shape, Shape, Shape> interfaces_only2;

	//
	// Here we'll use a pack in a static assertion:
	//
	template <typename... Ts>
	class InterfacesOnly3 {
		static_assert(satisfies<Ts...>(isInterfaceClass), "template arguments must be interface classes.");
		// implementation...
	};
	InterfacesOnly3<> interfaces_only3a;
	InterfacesOnly3<Shape> interfaces_only3b;
	InterfacesOnly3<Shape, Shape, Shape, Shape, Shape, Shape> interfaces_only3c;

	//
	// Here's an augmentation of the constraint;
	// is this type an interface class that is also derived from Shape?
	//
	constexpr auto isShapeInterfaceClass =
		isInterfaceClass & bind2nd<std::is_base_of, Shape>() & !bind2nd<std::is_same, Shape>();

	class WhatImpl {
	public:
		virtual ~WhatImpl() {}
	};

	template <typename T>
	class What : public WhatImpl {
	public:
		What() { std::cout << "What::What()" << std::endl; }

		//
		// Class interface use #1: tame greedy universal
		//     Only call universal version if S is "not similar" to T
		void doit(T const &a) { std::cout << "What::doit(lvalue): " << typeid(T).name() << std::endl; }
		void doit(T &&a) { std::cout << "What::doit(rvalue): " << typeid(T).name() << std::endl; }

		using NotSimilarToT = decltype(!bind2nd<are_similar, T>());
		template <typename S, typename = Constraint<NotSimilarToT, S>>
		void doit(S &&a) { std::cout << "What::doit(universal): " << typeid(S).name() << std::endl; }

		//
		// Class interface use #2: distinguish on secondary properties of args in an overload.
		//    Distinguish different stl iterator categories from each other and from raw pointers.
		template <typename In, typename = Constraint<decltype(isIn & !isRand), In>>
		What(In b, In e) { std::cout << "What::What(In, In)" << std::endl; }

		// Note that we have to add trailing default to allow member overload!
		// Here we're using Ignore, but any type will do.
		template <typename Rand, typename = Constraint<decltype(isRand & !isPointer), Rand>, typename = Ignore<1>>
		What(Rand b, Rand e) { std::cout << "What::What(Rand, Rand)" << std::endl; }

		template <typename Ptr, typename = Constraint<IsPointer, Ptr>, typename = Ignore<1>, typename = Ignore<2>>
		What(Ptr b, Ptr e) { std::cout << "What::What(Ptr, Ptr)" << std::endl; }

		//
		// Class interface use #3: distinguish on secondary properties of args in an overload.
		// Here we're using a constraint with an AST object (for convenience, so we don't need
		// to use decltype).
		//
		template <typename In, bool = constraint<In>(isIn & !isRand)>
		void f(In b, In e) const { std::cout << "What::f(In, In)" << std::endl; }

		// Have to add trailing default!  Here we're getting creative...
		template <typename Rand, bool = constraint<Rand>(isRand & !isPointer), typename = Ignore<1>>
		void f(Rand b, Rand e) const { std::cout << "What::f(Rand, Rand)" << std::endl; }

		template <typename Ptr, bool = constraint<Ptr>(isPointer), bool = ignore(1)>
		void f(Ptr b, Ptr e) const { std::cout << "What::f(Ptr, Ptr)" << std::endl; }

		//
		// Use #4:  add a constraint to an interface.
		template <typename Out, typename = Constraint<IsOut, Out>>
		void serialize(Out) {}

		static constexpr auto cast_constraint = isInterfaceClass & bind1st<std::is_base_of, Shape>() & !bind2nd<std::is_same, Shape>();
		using CastConstraint = decltype(cast_constraint);
		template <typename Target,
			      typename = Constraint<CastConstraint, Target>>
		Target *cast_it() { return reinterpret_cast<Target *>(this); }

		//
		// Use #5: tame overloaded universals, using Constraint.
		//
		template <typename... Ts, typename = Constraint<IsSigned, Ts...>>
		void justdoit(Ts &&... args) {
			std::cout << "Universal #1 signed: " << sizeof...(args)
				<< " Constraint (should be true): " << satisfies<Ts...>(isSigned) << std::endl;
		}

		template <typename... Ts, typename = Constraint<IsUnsigned, Ts...>, typename = void>
		void justdoit(Ts &&... args) {
			std::cout << "Universal #2 unsigned: " << sizeof...(args)
				<< " Constraint (should be true): " << satisfies<Ts...>(isUnsigned) << std::endl;
		}

		template <typename... Ts, typename = Constraint<IsClass, Ts...>, typename = void, typename = void>
		void justdoit(Ts &&... args) {
			std::cout << "Universal #3 class: " << sizeof...(args)
				<< " Constraint (should be true): " << satisfies<Ts...>(isClass) << std::endl;
		}

		//
		// Use #4: tame overloaded universals, using constraint.
		//
		template <typename... Ts, bool = constraint<Ts...>(isSigned)>
		void justdidit(Ts &&... args) {
			std::cout << "Universal #1 signed: " << sizeof...(args)
				<< " Constraint (should be true): " << satisfies<Ts...>(isSigned) << std::endl;
		}

		template <typename... Ts, bool = constraint<Ts...>(isUnsigned), typename = void>
		void justdidit(Ts &&... args) {
			std::cout << "Universal #2 unsigned: " << sizeof...(args)
				<< " Constraint (should be true): " << satisfies<Ts...>(isUnsigned) << std::endl;
		}
	};

	void testClassInterfaces() {
		using namespace std;

		cout << "====  CLASS INTERFACE  ====" << endl;

		What<Triangle> wt;
		Triangle triangle;
		Triangle const ctriangle;
		cout << "Should be rvalue: ";
		wt.doit(Triangle());
		cout << "Should be lvalue: ";
		wt.doit(triangle);
		cout << "Should be lvalue: ";
		wt.doit(ctriangle);
		cout << "Should be universal: ";
		wt.doit(1234);
		cout << "Should be universal: ";
		double *pd = 0;
		wt.doit(pd);

		cout << "Should be signed: ";
		wt.justdoit(12, 23, 45);
		cout << "Should be unsigned: ";
		wt.justdoit(sizeof(int), sizeof(char), sizeof(double), sizeof(long));
		cout << "Should be class: ";
		wt.justdoit(Circle(), Triangle());

		list<Triangle> lt;
		vector<Triangle> vt;
		Triangle at[3];

		cout << "Should be input: ";
		What<Triangle> wt2(lt.begin(), lt.end());
		cout << "Should be random access: ";
		What<Triangle> wt3(vt.begin(), vt.end());
		cout << "Should be pointer: ";
		What<Triangle> wt4(at, at + 3);

		cout << "Should be input: ";
		wt.f(lt.begin(), lt.end());
		cout << "Should be random access: ";
		wt.f(vt.begin(), vt.end());
		cout << "Should be pointer: ";
		wt.f(at, at + 3);

		wt.serialize(vt.begin());

		auto pp2 = wt.cast_it<Square>();
		cout << "Should be Square *: " << typeid(pp2).name() << endl;
	}

	void misc_tests() {
		using namespace std;

		cout << "====  MISC  ====" << endl;

		constexpr bool shape_is_interface = satisfies<Shape>(isInterfaceClass);
		cout << "Shape is interface (true): " << boolalpha << shape_is_interface << endl;

		constexpr bool circle_is_interface = satisfies<Circle>(isInterfaceClass);
		cout << "Circle is interface (false): " << boolalpha << circle_is_interface << endl;

		constexpr bool square_is_interface = isInterfaceClass.eval<Square>();
		cout << "Square is interface (true): " << boolalpha << isInterfaceClass.eval<Square>() << endl;

		constexpr bool triangle_is_interface = satisfies<Triangle>(isInterfaceClass);
		cout << "Triangle is interface (false): " << boolalpha << satisfies<Triangle>(isInterfaceClass) << endl;

		constexpr bool shape_is_big_interface = satisfies<Shape>(isInterfaceClass & !isSmall);
		cout << "Shape is big interface (false): " << boolalpha << shape_is_big_interface << endl;

		constexpr bool square_is_big_interface = satisfies<Square>(isInterfaceClass & !isSmall);
		cout << "Square is big interface (false): " << boolalpha << square_is_big_interface << endl;

		constexpr auto p2 = pred<is_polymorphic>() | pred<is_signed>();
		constexpr bool r2 = satisfies<int>(p2);
		cout << "int is polymorphic or signed (true): " << boolalpha << r2 << endl;

		// A nonsensical but complex AST:
		constexpr auto too_complex =
			(isClass & (isPod | !isPolymorphic) ^ isUnsigned & !isTrivial)
			| (isPtrSize & !isFloatingPoint)
			| bind2nd<is_of_size, 8>();
		constexpr bool r3 = satisfies<Circle>(too_complex);
		cout << "Circle is too_complex (true): " << boolalpha << r3 << endl;
		constexpr bool r4 = satisfies<int>(too_complex);
		cout << "int is too_complex (true): " << boolalpha << r4 << endl;
		constexpr bool r5 = satisfies<float>(too_complex);
		cout << "float is too_complex (false): " << boolalpha << r5 << endl;
		constexpr bool r6 = satisfies<unsigned long long>(too_complex);
		cout << "unsigned long long is too_complex (true): " << boolalpha << r6 << endl;
	}

}

template <typename T>
void test_binders() {
	using namespace std;
	using namespace DandS;

	cout << "====  BINDERS  ====" << endl;

	constexpr auto r1 = satisfies<T>(pred<is_pod>());
	cout << typeid(T).name() << " is a pod: " << boolalpha << r1 << endl;

	constexpr auto p2 = bind1st<is_base_of, Shape>();
	constexpr auto r2 = satisfies<T>(p2);
	cout << typeid(T).name() << " is a Shape: " << boolalpha << r2 << endl;

	constexpr auto shapeish = bind1st<is_base_of, Shape>() | bind2nd<is_base_of, Shape>();
	constexpr auto r3 = satisfies<T>(shapeish);
	cout << typeid(T).name() << " is a Shape or a Shape is it: " << boolalpha << r3 << endl;
}

template <typename T>
void test_expressions() {
	using namespace std;
	using namespace DandS;

	cout << "====  EXPRESSIONS  ====" << endl;

	constexpr auto r1 = satisfies<T>(isStandardLayout & !isPod);
	cout << typeid(T).name() << " is standard layout but not pod: " << boolalpha << r1 << endl;

	constexpr auto isInterfaceClass =
		isPtrSize &
		hasVirtualDestructor &
		isAbstract;
	constexpr auto r2 = satisfies<T>(isInterfaceClass);
	cout << typeid(T).name() << " is interface class: " << boolalpha << r2 << endl;

	constexpr auto isConcreteShape = bind1st<is_base_of, Shape>() & !isAbstract;
	constexpr auto r3 = satisfies<T>(isConcreteShape);
	cout << typeid(T).name() << " is concrete Shape: " << boolalpha << r3 << endl;
}

int main() {
	try {
		using namespace DandS;

		test_binders<double>();
		test_binders<Circle>();
		test_binders<ShapeBase>();
		test_expressions<double>();
		test_expressions<Circle>();
		test_expressions<Shape>();
		test_expressions<ShapeBase>();
		testClassInterfaces();
		misc_tests();
	}
	catch (const std::exception &e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}
	catch (...) {
		std::cout << "Unknown exception" << std::endl;
	}

	return 0;
}

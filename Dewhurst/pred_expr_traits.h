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

#ifndef INCLUDED_PRED_EXPR_TRAITS
#define INCLUDED_PRED_EXPR_TRAITS

#include "pred_expr.h"
#include <type_traits>

// Convenience representation of standard type traits as AST ids.
//
namespace DandS {
	
	constexpr auto isVoid = pred<std::is_void>();
	using IsVoid = decltype(isVoid);
	constexpr auto isIntegral = pred<std::is_integral>();
	using IsIntegral = decltype(isIntegral);
	constexpr auto isFloatingPoint = pred<std::is_floating_point>();
	using IsFloatingPoint = decltype(isFloatingPoint);
	constexpr auto isArithmetic = pred<std::is_arithmetic>();
	using IsArithmetic = decltype(isArithmetic);
	constexpr auto isSigned = pred<std::is_signed>();
	using IsSigned = decltype(isSigned);
	constexpr auto isUnsigned = pred<std::is_unsigned>();
	using IsUnsigned = decltype(isUnsigned);
	constexpr auto isConst = pred<std::is_const>();
	using IsConst = decltype(isConst);
	constexpr auto isVolatile = pred<std::is_volatile>();
	using IsVolatile = decltype(isVolatile);
	constexpr auto isArray = pred<std::is_array>();
	using IsArray = decltype(isArray);
	constexpr auto isEnum = pred<std::is_enum>();
	using IsEnum = decltype(isEnum);
	constexpr auto isUnion = pred<std::is_union>();
	using IsUnion = decltype(isUnion);
	constexpr auto isClass = pred<std::is_class>();
	using IsClass = decltype(isClass);
	constexpr auto isFunction = pred<std::is_function>();
	using IsFunction = decltype(isFunction);
	constexpr auto isReference = pred<std::is_reference>();
	using IsReference = decltype(isReference);
	constexpr auto isLvalueReference = pred<std::is_lvalue_reference>();
	using IsLvalueReference = decltype(isLvalueReference);
	constexpr auto isRvalueReference = pred<std::is_rvalue_reference>();
	using IsRvalueReference = decltype(isRvalueReference);
	constexpr auto isPointer = pred<std::is_pointer>();
	using IsPointer = decltype(isPointer);
	constexpr auto isMemberPointer = pred<std::is_member_pointer>();
	using IsMemberPointer = decltype(isMemberPointer);
	constexpr auto isMemberObjectPointer = pred<std::is_member_object_pointer>();
	using IsMemberObjectPointer = decltype(isMemberObjectPointer);
	constexpr auto isFundamental = pred<std::is_fundamental>();
	using IsFundamental = decltype(isFundamental);
	constexpr auto isScalar = pred<std::is_scalar>();
	using IsScalar = decltype(isScalar);
	constexpr auto isObject = pred<std::is_object>();
	using IsObject = decltype(isObject);
	constexpr auto isCompound = pred<std::is_compound>();
	using IsCompound = decltype(isCompound);
	constexpr auto isTrivial = pred<std::is_trivial>();
	using IsTrivial = decltype(isTrivial);
	constexpr auto isTriviallyCopyable = pred<std::is_trivially_copyable>();
	using IsTriviallyCopyable = decltype(isTriviallyCopyable);
	constexpr auto isStandardLayout = pred<std::is_standard_layout>();
	using IsStandardLayout = decltype(isStandardLayout);
	constexpr auto isPod = pred<std::is_pod>();
	using IsPod = decltype(isPod);
	constexpr auto isLiteralType = pred<std::is_literal_type>();
	using IsLiteralType = decltype(isLiteralType);
	constexpr auto isEmpty = pred<std::is_empty>();
	using IsEmpty = decltype(isEmpty);
	constexpr auto isPolymorphic = pred<std::is_polymorphic>();
	using IsPolymorphic = decltype(isPolymorphic);
	constexpr auto isAbstract = pred<std::is_abstract>();
	using IsAbstract = decltype(isAbstract);
	constexpr auto hasVirtualDestructor = pred<std::has_virtual_destructor>();
	using HasVirtualDestructor = decltype(hasVirtualDestructor);
	constexpr auto isDefaultConstructible = pred<std::is_default_constructible>();
	using IsDefaultConstructible = decltype(isDefaultConstructible);
	constexpr auto isCopyConstructible = pred<std::is_copy_constructible>();
	using IsCopyConstructible = decltype(isCopyConstructible);
	constexpr auto isMoveConstructible = pred<std::is_move_constructible>();
	using IsMoveConstructible = decltype(isMoveConstructible);
	constexpr auto isCopyAssignable = pred<std::is_copy_assignable>();
	using IsCopyAssignable = decltype(isCopyAssignable);
	constexpr auto isMoveAssignable = pred<std::is_move_assignable>();
	using IsMoveAssignable = decltype(isMoveAssignable);
	constexpr auto isDestructible = pred<std::is_destructible>();
	using IsDestructible = decltype(isDestructible);
	constexpr auto isTriviallyDefaultConstructible = pred<std::is_trivially_default_constructible>();
	using IsTriviallyDefaultConstructible = decltype(isTriviallyDefaultConstructible);
	constexpr auto isTriviallyCopyConstructible = pred<std::is_trivially_copy_constructible>();
	using IsTriviallyCopyConstructible = decltype(isTriviallyCopyConstructible);
	constexpr auto isTriviallyMoveConstructible = pred<std::is_trivially_move_constructible>();
	using IsTriviallyMoveConstructible = decltype(isTriviallyMoveConstructible);
	constexpr auto isTriviallyCopyAssignable = pred<std::is_trivially_copy_assignable>();
	using IsTriviallyCopyAssignable = decltype(isTriviallyCopyAssignable);
	constexpr auto isTriviallyMoveAssignable = pred<std::is_trivially_move_assignable>();
	using IsTriviallyMoveAssignable = decltype(isTriviallyMoveAssignable);
	constexpr auto isNothrowDefaultConstructible = pred<std::is_nothrow_default_constructible>();
	using IsNothrowDefaultConstructible = decltype(isNothrowDefaultConstructible);
	constexpr auto isNothrowCopyConstructible = pred<std::is_nothrow_copy_constructible>();
	using IsNothrowCopyConstructible = decltype(isNothrowCopyConstructible);
	constexpr auto isNothrowMoveConstructible = pred<std::is_nothrow_move_constructible>();
	using IsNothrowMoveConstructible = decltype(isNothrowMoveConstructible);
	constexpr auto isNothrowCopyAssignable = pred<std::is_nothrow_copy_assignable>();
	using IsNothrowCopyAssignable = decltype(isNothrowCopyAssignable);
	constexpr auto isNothrowMoveAssignable = pred<std::is_nothrow_move_assignable>();
	using IsNothrowMoveAssignable = decltype(isNothrowMoveAssignable);
	constexpr auto isNothrowDestructible = pred<std::is_nothrow_destructible>();
	using IsNothrowDestructible = decltype(isNothrowDestructible);

}

#endif

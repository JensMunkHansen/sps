#pragma once

namespace sps {

/********** std::remove_cv replacement **********/
template< typename T >
struct remove_const {
  typedef T type;
};

template< typename T >
struct remove_const< const T > {
  typedef T type;
};

template< typename T >
struct remove_volatile {
  typedef T type;
};

template< typename T >
struct remove_volatile< volatile T > {
  typedef T type;
};


template< typename T >
struct remove_cv {
  typedef typename remove_volatile<typename remove_const<T>::type >::type type;
};


/********** std::is_pointer replacement *********/
template< typename T >
struct is_pointer_helper {
  static const bool value = false;
};

template< typename T >
struct is_pointer_helper< T* > {
  static const bool value = true;
};

template< typename T >
struct is_pointer {
  static const bool value = is_pointer_helper< typename remove_cv< T >::type >::value;
};


/********** std::enable_if replacement **********/
template< bool CONDITION, typename TYPE = void >
struct enable_if {
};

template< typename TYPE >
struct enable_if< true, TYPE > {
  typedef TYPE type;
};


/****** std::remove_reference replacement *******/
template< typename T >
struct remove_reference {
  typedef T type;
};

template< typename T >
struct remove_reference< T& > {
  typedef T type;
};


/******* std::is_constructible replacement ******/
template< typename T, typename AT_1 = void, typename AT_2 = void, typename AT_3 = void, typename AT_4 = void >
class is_constructible_impl {
 private:
  template< typename C_T, typename C_AT_1,
            typename C_AT_2, typename C_AT_3, typename C_AT_4 >
  static bool test(
      typename sps::enable_if<
      sizeof( C_T ) ==
      sizeof( C_T(
          static_cast< C_AT_1 >( *static_cast< typename sps::remove_reference< C_AT_1 >::type* >( NULL ) ),
          static_cast< C_AT_2 >( *static_cast< typename sps::remove_reference< C_AT_2 >::type* >( NULL ) ),
          static_cast< C_AT_3 >( *static_cast< typename sps::remove_reference< C_AT_3 >::type* >( NULL ) ),
          static_cast< C_AT_4 >( *static_cast< typename sps::remove_reference< C_AT_4 >::type* >( NULL ) )
                  ))
      >::type*
                   );

  template< typename, typename, typename, typename, typename >
  static int test( ... );
 public:
  static const bool value = ( sizeof( test< T, AT_1, AT_2, AT_3, AT_4 >( NULL ) ) == sizeof( bool ) );
};

template< typename T, typename AT_1, typename AT_2, typename AT_3 >
class is_constructible_impl< T, AT_1, AT_2, AT_3, void > {
 private:
    template< typename C_T, typename C_AT_1, typename C_AT_2, typename C_AT_3 >
    static bool test(
        typename sps::enable_if<
            sizeof( C_T ) ==
            sizeof( C_T(
                static_cast< C_AT_1 >( *static_cast< typename sps::remove_reference< C_AT_1 >::type* >( NULL ) ),
                static_cast< C_AT_2 >( *static_cast< typename sps::remove_reference< C_AT_2 >::type* >( NULL ) ),
                static_cast< C_AT_3 >( *static_cast< typename sps::remove_reference< C_AT_3 >::type* >( NULL ) )
            ) )
        >::type*
    );

    template< typename, typename, typename, typename >
    static int test( ... );

public:
    static const bool value = ( sizeof( test< T, AT_1, AT_2, AT_3 >( NULL ) ) == sizeof( bool ) );
};

template< typename T, typename AT_1, typename AT_2 >
class is_constructible_impl< T, AT_1, AT_2, void, void > {
 private:
    template< typename C_T, typename C_AT_1, typename C_AT_2 >
    static bool test(
        typename sps::enable_if<
            sizeof( C_T ) ==
            sizeof( C_T(
                static_cast< C_AT_1 >( *static_cast< typename sps::remove_reference< C_AT_1 >::type* >( NULL ) ),
                static_cast< C_AT_2 >( *static_cast< typename sps::remove_reference< C_AT_2 >::type* >( NULL ) )
            ) )
        >::type*
    );

    template< typename, typename, typename >
    static int test( ... );

public:
    static const bool value = ( sizeof( test< T, AT_1, AT_2 >( NULL ) ) == sizeof( bool ) );
};

template< typename T, typename AT_1 >
class is_constructible_impl< T, AT_1, void, void, void >
{
private:
    template< typename C_T, typename C_AT_1 >
    static bool test(
        typename sps::enable_if<
            sizeof( C_T ) ==
            sizeof( C_T(
                static_cast< C_AT_1 >( *static_cast< typename sps::remove_reference< C_AT_1 >::type* >( NULL ) )
            ) )
        >::type*
    );

    template< typename, typename >
    static int test( ... );

public:
    static const bool value = ( sizeof( test< T, AT_1 >( NULL ) ) == sizeof( bool ) );
};

template< typename T >
class is_constructible_impl< T, void, void, void, void >
{
private:
    template< typename C_T >
    static C_T testFun( C_T );

    template< typename C_T >
    static bool test( typename sps::enable_if< sizeof( C_T ) == sizeof( testFun( C_T() ) ) >::type* );

    template< typename >
    static int test( ... );

public:
    static const bool value = ( sizeof( test< T >( NULL ) ) == sizeof( bool ) );
};

template< typename T, typename AT_1 = void, typename AT_2 = void, typename AT_3 = void, typename AT_4 = void >
class is_constructible_impl_ptr
{
public:
    static const bool value = false;
};

template< typename T, typename AT_1 >
class is_constructible_impl_ptr< T, AT_1, typename enable_if< is_pointer< typename remove_reference< T >::type >::value, void >::type, void, void >
{
private:
    template< typename C_T >
    static bool test( C_T );

    template< typename >
    static int test( ... );

public:
    static const bool value = ( sizeof( test< T >( static_cast< AT_1 >( NULL ) ) ) == sizeof( bool ) );
};

template< typename T >
class is_constructible_impl_ptr< T, void, void, void, void >
{
public:
    static const bool value = true;
};

template< typename T, typename AT_1 = void, typename AT_2 = void, typename AT_3 = void, typename AT_4 = void >
class is_constructible {
 public:
  static const bool value = (
      is_pointer< typename remove_reference< T >::type >::value ?
      is_constructible_impl_ptr< T, AT_1, AT_2, AT_3, AT_4 >::value :
      is_constructible_impl< T, AT_1, AT_2, AT_3, AT_4 >::value);
};

template<class T>
struct is_copy_constructible :
      std::is_constructible<T, typename std::add_lvalue_reference<
                                 typename std::add_const<T>::type>::type>> {};

template<class T>
struct is_trivially_copy_constructible :
      std::is_trivially_constructible<T, typename std::add_lvalue_reference<
                                           typename std::add_const<T>::type>::type>> {};

template<class T>
struct is_nothrow_copy_constructible :
      std::is_nothrow_constructible<T, typename std::add_lvalue_reference<
                                         typename std::add_const<T>::type>::type>> {};
}  // namespace sps

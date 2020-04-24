#include <iostream>
#include <type_traits>
//#include "Event.hpp"
//
//void f1(int a)
//{
//  std::cout << "a " << a << std::endl;
//}
//
//void f2(int a)
//{
//  std::cout << "b " << a << std::endl;
//}
//
//void f3(int a)
//{
//  std::cout << "c " << a << std::endl;
//}
//
//struct A
//{
//  static void sf(int a)
//  {
//    std::cout << "sf " << a << std::endl;
//  }
//
//  void mf(int a)
//  {
//    std::cout << "mf " << a << std::endl;
//  }
//};
//
//Event<void(int)> e;
//
//int main()
//{
//  try
//  {
//    A a;
//    e.Hook(f1);
//    e.Hook(f2);
//    e.Hook(f3);
//    e.Hook(&a, &A::mf);
//
//    e.Invoke(0);
//
//  }
//  catch (const char *str)
//  {
//    std::cout << str;
//  }
//	return 0;
//}
//#define VERIFY_TYPE noexcept
//
//template<typename Invalid_Type, typename ...Types>
//static constexpr bool type_exclusion() // assures a type is not contained within Types...
//{
//  static_assert(sizeof ...(Types) > 0, "Calling variadic function with no parameters");
//  static_assert(!std::disjunction_v<std::is_same<Invalid_Type, Types>...>, "Invalid Type contained in Types variadic list");
//  return true;
//}
//
//template<typename C, typename ...Fs>
//static constexpr bool class_member_inclusion() // assures Fs... are members of class C
//{
//  static_assert(std::is_class<C>::value, "Provided class pointer is not a class");
//  static_assert(sizeof ...(Fs) > 0, "Calling variadic function with no parameters");
//  static_assert((std::is_member_function_pointer<Fs>::value, ...), "A function in variadic list Fs... is not a member of class C");
//  return true;
//}
//
//template<typename A, typename B>
//constexpr void IsMember(typename A::B cc)
//{
//
//}
//
//template<typename ...Fs>
//static constexpr bool class_member_exclusion() // assures Fs... are not members of a class
//{
//  static_assert(sizeof ...(Fs) > 0, "Calling variadic function with no parameters");
//  static_assert(!std::disjunction_v<std::is_member_function_pointer<Fs>...>, "A function in variadic list Fs... is a class member");
//  return true;
//}
//
////template<typename ...T>
////void function(T... ts) VERIFY_TYPE(type_exclusion<float, T...>())
////{}
//
//template<typename C, typename ...T>
//void function(C *c, T... ts) VERIFY_TYPE(class_member_inclusion<C, T...>())
//{}
//
////template<typename ...T>
////void function(T... ts) VERIFY_TYPE(class_member_exclusion<T...>())
////{}
//
//void function1(){}
//void function2(){}
//void function3(){}
//
//struct Cl
//{
//  void method1(){}
//  void method2(){}
//  static void method3(){}
//};
//
//struct Cl2
//{
//  void method1(){}
//  void method2(){}
//  static void method3(){}
//};
//
//void f1(){
//}
//
//struct C1 {
//  void m1() {}
//  static void m6() {}
//};
//
//struct C2 {
//  void m2(){}
//};

//template<typename T, typename C, typename R, typename ...Args>
//constexpr bool IsMemberFunction(R(C::*)(Args...))
//{
//  return std::is_same<T, C>::value;
//}
//
//template<typename C, typename ...F>
//constexpr bool Members(F... fs)
//{
//  static_assert(std::is_class<C>::value, "Class pointer provided not of a class type");
//  static_assert((... && (std::is_member_function_pointer<F>::value && IsMemberFunction<C>(fs))), "Class pointer provided not of a class type");
//  return true;
//}

//template<typename, typename>
//struct is_member_function_of
//{
//  static constexpr bool value = false;
//};
//
//#define DEF_IS_MEMBER_FUNCTION_OF(CV_REF_NOEXCEPT_OPT) \
//    template<typename T, typename Ret, typename... Args> \
//    struct is_member_function_of<T, Ret (T::*)(Args...) CV_REF_NOEXCEPT_OPT> \
//    { \
//        static constexpr bool value = true; \
//    }
//
//#define DEF_IS_MEMBER_FUNCTION_OF_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
//    template<typename T, typename Ret, typename... Args> \
//    struct is_member_function_of<T, Ret (T::*)(Args..., ...) CV_REF_NOEXCEPT_OPT> \
//    { \
//        static constexpr bool value = true; \
//    }
//
//#define PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_REF_OPT) \
//    MACRO(CV_REF_OPT); \
//    MACRO(CV_REF_OPT noexcept)
//
//#define PERMUTE_PMF_CV_REF(MACRO, CV_OPT) \
//    PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT); \
//    PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT &); \
//    PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT &&)
//
//#define PERMUTE_PMF_CV(MACRO) \
//    PERMUTE_PMF_CV_REF(MACRO, ); \
//    PERMUTE_PMF_CV_REF(MACRO, const); \
//    PERMUTE_PMF_CV_REF(MACRO, volatile); \
//    PERMUTE_PMF_CV_REF(MACRO, const volatile)
//
//#define PERMUTE_PMF(MACRO) \
//    PERMUTE_PMF_CV(MACRO); \
//    PERMUTE_PMF_CV(MACRO##_ELLIPSIS)
//
//PERMUTE_PMF(DEF_IS_MEMBER_FUNCTION_OF);
//
//template<typename T, typename Fn>
//constexpr bool is_member_function_of_v = is_member_function_of<T, Fn>::value;

#include "Event.hpp"

struct C1 {
  void m1() { std::cout << "??" << std::endl; }
  static void m6() {}
};

struct C2 {
  void m2(){}
};

Event<void(void)> e;

void f1(){
  std::cout << "F1" << std::endl;
}

void f() {
  e.Hook(f1);
}

int main(void)
{
  C1 c1;
  C2 c2;
  e.Hook(f1);
  e.Hook(c1, f1);
  e.Invoke();
//
//
//  template<typename C, typename ...Fs>
//  void function(C *class_ptr, Fs... func_ptrs)
//  VERIFY_TYPE(class_member_inclusion<C, Fs...>())
//  {}
//
//  Cl c;
//  Cl2 c2;
//  function(&c, &Cl::method1);
//  function(function1, Cl::method3);

  return 0;
}
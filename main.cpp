//#include <iostream>
//#include "Events.hpp"
//
//struct Struct
//{
//  static void static_function(int a)
//  {
//    std::cout << "Hello from static member function = " << a << std::endl;
//  }
//
//  void member_function(int a)
//  {
//    std::cout << "Hello from member function = " << a << std::endl;
//  }
//};
//
//void function(int a)
//{
//  std::cout << "Hello from function = " << a << std::endl;
//}
//
//int main()
//{
//  // uncomment for keeping order
//  Event<void(int) /*, true */> event;
//  Struct aStruct;
//
//  // Hook them
//  event.Hook(Struct::static_function);
//  event.Hook(aStruct, &Struct::member_function);
//  event.Hook(function);
//  EVENT_HANDLE lamda_handle = event.Hook([](int a){ std::cout << "Hello from lambda = " << a << std::endl; });
//
//  // Say hello
//  event.Invoke(1);
//
//  // Unhook them
//  event.Unhook(Struct::static_function);
//  event.UnhookClass(aStruct); // or event.Unhook(aStruct, &Struct::member_function);
//  event.Unhook(function);
//  event.Unhook(lamda_handle);
//
//  // Say nothing
//  event.Invoke(1);
//
//  return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//struct object
//{
//    void method(int val)
//    {
//        std::cout << "Value for method is " << val << std::endl;
//    }
//};
//
//void function(int val)
//{
//    std::cout << "Value for function is " << val << std::endl;
//}
//
//int main(void)
//{
//    // Create
//    Event<void(int)> event;
//    object obj;
//
//    // Hook
//    event.Hook(obj, &object::method);
//    event.Hook(function);
//    EVENT_HANDLE handle = event.Hook([](int val) {
//        std::cout << "Value for lambda is " << val << std::endl;
//    });
//
//    // Invoke
//    event.Invoke(123);
//
//
//    // Unhook
//    event.UnhookClass(obj);
//    event.Unhook(handle);
//    event.Unhook(function);
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//
//    std::cout << "Call list size is " << event.CallListSize() << std::endl;
//
//    // Hook
//    EVENT_HANDLE handle_1 = event.Hook([](){});
//    EVENT_HANDLE handle_2 = event.Hook([](){});
//
//    std::cout << "Call list size is " << event.CallListSize() << std::endl;
//
//    // Unhook
//    event.Unhook(handle_1);
//
//    std::cout << "Call list size is " << event.CallListSize() << std::endl;
//
//    // Unhook
//    event.Unhook(handle_2);
//
//    std::cout << "Call list size is " << event.CallListSize() << std::endl;
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//struct object
//{
//    void method()
//    {
//        std::cout << "Method" << std::endl;
//    }
//
//    static void static_method()
//    {
//        std::cout << "Static Method" << std::endl;
//    }
//};
//
//void function()
//{
//    std::cout << "Function" << std::endl;
//}
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//    object obj;
//
//    // Hook
//    EVENT_HANDLE handle = event.Hook([](){ std::cout << "Lambda" << std::endl; });
//    event.Hook(function);
//    event.Hook(object::static_method);
//    event.Hook(obj, &object::method);
//
//    // Invoke
//    event.Invoke();
//
//
//    // Unhook
//    event.Unhook(handle);
//    event.Unhook(function);
//    event.Unhook(object::static_method);
//    event.Unhook(obj, &object::method);
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//void function1() { std::cout << "function1" << std::endl; }
//void function2() { std::cout << "function2" << std::endl; }
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//
//    // Hook
//    EVENT_HANDLE handle = event.HookFunctionCluster(function1, function2, [](){ std::cout << "lambda1" << std::endl; }, [](){ std::cout << "lambda2" << std::endl; });
//
//    // Invoke
//    event.Invoke();
//
//    // Unhook
//    event.UnhookCluster(handle);
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//struct object
//{
//    void method1()
//    {
//        std::cout << "Method 1" << std::endl;
//    }
//
//    void method2()
//    {
//        std::cout << "Method 2" << std::endl;
//    }
//};
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//    object obj;
//
//    // Hook
//    EVENT_HANDLE handle = event.HookMethodCluster(obj,
//                                                  &object::method1,
//                                                  &object::method2);
//
//    // Invoke
//    event.Invoke();
//
//    // Unhook
//    event.Unhook(handle);
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//struct object
//{
//    void method()
//    {
//        std::cout << "Method" << std::endl;
//    }
//
//    static void static_method()
//    {
//        std::cout << "Static Method" << std::endl;
//    }
//};
//
//void function()
//{
//    std::cout << "Function" << std::endl;
//}
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//    object obj;
//
//    // Hook
//    EVENT_HANDLE handle = event.Hook([](){ std::cout << "Lambda" << std::endl; });
//    event.Hook(function);
//    event.Hook(object::static_method);
//    event.Hook(obj, &object::method);
//
//    std::cout << "Call list size is " << event.CallListSize() << std::endl;
//
//    // Invoke
//    event.Invoke();
//
//    // Unhook
//    event.Unhook(handle);
//    event.Unhook(function);
//    event.Unhook(object::static_method);
//    event.Unhook(obj, &object::method);
//
//    // Invoke nothing
//    event.Invoke();
//
//    std::cout << "Call list size is " << event.CallListSize() << std::endl;
//
//    return 0;
//}


//#include "Events.hpp"
//#include <iostream>
//
//struct object
//{
//    void method1()
//    {
//        std::cout << "Method 1" << std::endl;
//    }
//
//    void method2()
//    {
//        std::cout << "Method 2" << std::endl;
//    }
//};
//
//void function1()
//{
//    std::cout << "Function 1" << std::endl;
//}
//
//void function2()
//{
//    std::cout << "Function 2" << std::endl;
//}
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//    object obj;
//
//    // Hook
//    EVENT_HANDLE handle_m = event.HookMethodCluster(obj, &object::method1, &object::method2);
//    EVENT_HANDLE handle_f = event.HookFunctionCluster(function1, function2,
//                                                      [](){ std::cout << "Lambda 1" << std::endl; },
//                                                      [](){ std::cout << "Lambda 2" << std::endl; });
//
//    std::cout << "Size of call list is " << event.CallListSize() << std::endl;
//
//    // Invoke
//    event.Invoke();
//
//    // Unhook
//    event.UnhookCluster(handle_m);
//    event.UnhookCluster(handle_f);
//
//    std::cout << "Size of call list is " << event.CallListSize() << std::endl;
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//struct object
//{
//    void method1()
//    {
//        std::cout << "Method 1" << std::endl;
//    }
//
//    void method2()
//    {
//        std::cout << "Method 2" << std::endl;
//    }
//};
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//    object obj;
//
//    // Hook
//    event.Hook(obj, &object::method1);
//    event.Hook(obj, &object::method2);
//
//    std::cout << "Size of call list is " << event.CallListSize() << std::endl;
//
//    // Invoke
//    event.Invoke();
//
//    // Unhook
//    event.UnhookClass(obj);
//
//    std::cout << "Size of call list is " << event.CallListSize() << std::endl;
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//void function1()
//{
//    std::cout << "Function 1" << std::endl;
//}
//
//void function2()
//{
//    std::cout << "Function 2" << std::endl;
//}
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//
//    // Hook
//    event.Hook(function1);
//    event.Hook(function2);
//
//    std::cout << "Size of call list is " << event.CallListSize() << std::endl;
//
//    // Invoke
//    event.Invoke();
//
//    // Unhook
//    event.UnhookFunctions(function1, function2);
//
//    std::cout << "Size of call list is " << event.CallListSize() << std::endl;
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//struct object
//{
//    void method1()
//    {
//        std::cout << "Method 1" << std::endl;
//    }
//
//    void method2()
//    {
//        std::cout << "Method 2" << std::endl;
//    }
//};
//
//int main(void)
//{
//    // Create
//    Event<void(void)> event;
//    object obj;
//
//    // Hook
//    event.Hook(obj, &object::method1);
//    event.Hook(obj, &object::method2);
//
//    std::cout << "Size of call list is " << event.CallListSize() << std::endl;
//
//    // Invoke
//    event.Invoke();
//
//    // Unhook
//    event.UnhookMethods(obj, &object::method1, &object::method2);
//
//    std::cout << "Size of call list is " << event.CallListSize() << std::endl;
//
//    return 0;
//}

//#include "Events.hpp"
//#include <iostream>
//
//struct object
//{
//    void method(int val)
//    {
//        std::cout << "Value for method is " << val << std::endl;
//    }
//};
//
//void function(int val)
//{
//    std::cout << "Value for function is " << val << std::endl;
//}
//
//int main(void)
//{
//    // Create
//    Event<void(int)> event;
//    object obj;
//
//    // Hook
//    event.Hook(obj, &object::method);
//    event.Hook(function);
//    EVENT_HANDLE handle = event.Hook([](int val) {
//        std::cout << "Value for lambda is " << val << std::endl;
//    });
//
//    std::cout << "Call list size is " << event.CallListSize() << std::endl;
//
//    // Invoke
//    event.Invoke(123);
//
//    // Clear
//    event.Clear();
//
//    std::cout << "Call list size is " << event.CallListSize() << std::endl;
//
//    return 0;
//}

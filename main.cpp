#include <iostream>
#include "Event.hpp"

struct Struct
{
  static void static_function(int a)
  {
    std::cout << "Hello from static member function = " << a << std::endl;
  }

  void member_function(int a)
  {
    std::cout << "Hello from member function = " << a << std::endl;
  }
};

void function(int a)
{
  std::cout << "Hello from function = " << a << std::endl;
}

int main()
{
  // uncomment for keeping order
  Event<void(int) /*, true */> event;
  Struct aStruct;

  // Hook them
  event.Hook(Struct::static_function);
  event.Hook(aStruct, &Struct::member_function);
  event.Hook(function);
  EVENT_HANDLE lamda_handle = event.Hook([](int a){ std::cout << "Hello from lambda = " << a << std::endl; });

  // Say hello
  event.Invoke(1);

  // Unhook them
  event.Unhook(Struct::static_function);
  event.UnhookClass(aStruct); // or event.Unhook(aStruct, &Struct::member_function);
  event.Unhook(function);
  event.Unhook(lamda_handle);

  // Say nothing
  event.Invoke(1);

  return 0;
}
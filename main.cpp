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
  Event<void(int)> event;
  Struct aStruct;
  event.Hook(Struct::static_function);
  event.Hook(aStruct, &Struct::member_function);
  event.Hook(function);
  event.Hook([](int a){ std::cout << "Hello from lambda = " << a << std::endl; });
  event.Invoke(1);
  return 0;
}
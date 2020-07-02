# Hook
#### Event<FunctionSignature, KeepOrder, Allocator>::___Hook___

-----

__template\<typename Fn\>  
  EVENT_HANDLE Hook(Fn &&func_ptr);__
  
__template\<typename C, typename Fn\>  
  EVENT_HANDLE Hook(C &class_ref, Fn func_ptr);__

Hooks a method or function to the event to be invoked

##### Parameters
__`class_ref`__ - Reference to the class that contains attempted hooked method 'func_ptr'

__`func_ptr`__ - Address to method, function or lambda to be hooked

##### Return value
An EVENT_HANDLE to the corresponding hooked function

##### Complexity
O(1)

##### Notes
In order to unhook a lambda, the only way to do so is through its EVENT_HANDLE.  
If order of invocation does not matter relative to hooking order, consider setting template parameter __`KeepOrder`__ to false. 


##### Example
```c++
#include "Events.hpp"
#include <iostream>

struct object
{
    void method()
    {
        std::cout << "Method" << std::endl;
    }

    static void static_method()
    {
        std::cout << "Static Method" << std::endl;
    }
};

void function()
{
    std::cout << "Function" << std::endl;
}

int main(void)
{
    // Create
    Event<void(void) /*, false*/> event;
    object obj;

    // Hook
    EVENT_HANDLE handle = event.Hook([](){ std::cout << "Lambda" << std::endl; });
    event.Hook(function);
    event.Hook(object::static_method);
    event.Hook(obj, &object::method);

    // Invoke
    event.Invoke();


    // Unhook
    event.Unhook(handle);
    event.Unhook(function);
    event.Unhook(object::static_method);
    event.Unhook(obj, &object::method);
    
    return 0;
}
```

Possible output:

```c++17
Lambda
Function
Static Method
Method
```
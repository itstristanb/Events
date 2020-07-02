# Unhook
#### Event<FunctionSignature, KeepOrder, Allocator>::___Unhook___

-----

__template\<typename Fn\>  
  void Unhook(Fn func_ptr);__
  
__template\<typename C, typename Fn\>  
  void Unhook(C &class_ref, Fn func_ptr);__

__void Unhook(EVENT_HANDLE handle)__

Removes a function, lambda or method from the call list and being invoked

##### Parameters
__`func_ptr`__ - Address to the function or method to be unhooked

__`class_ref`__ - Reference to the class containing the method 'func_ptr'

__`handle`__ - Handle returned by the event's [Hook](https://github.com/BeOurQuest/Events/wiki/Hook)

##### Return value
(none)

##### Complexity
if  __`KeepOrder`__ is true, O(N) where N is the size of the call list  
if  __`KeepOrder`__ is false, O(1)

##### Notes
This function will NOT work with handles returned by [HookFunctionCluster](https://github.com/BeOurQuest/Events/wiki/HookFunctionCluster) or [HookMethodCluster](https://github.com/BeOurQuest/Events/wiki/HookMethodCluster).  Use [UnhookCluster](https://github.com/BeOurQuest/Events/wiki/UnhookCluster).  
If many methods are hooked from the same class. Consider [UnhookClass](https://github.com/BeOurQuest/Events/wiki/UnhookClass) or [UnhookMethods](https://github.com/BeOurQuest/Events/wiki/UnhookMethods).  
If many functions need to be unhooked at once. Consider [UnhookFunctions](https://github.com/BeOurQuest/Events/wiki/UnhookFunctions).

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
    Event<void(void)> event;
    object obj;

    // Hook
    EVENT_HANDLE handle = event.Hook([](){ std::cout << "Lambda" << std::endl; });
    event.Hook(function);
    event.Hook(object::static_method);
    event.Hook(obj, &object::method);

    std::cout << "Call list size is " << event.CallListSize() << std::endl;

    // Invoke
    event.Invoke();

    // Unhook
    event.Unhook(handle);
    event.Unhook(function);
    event.Unhook(object::static_method);
    event.Unhook(obj, &object::method);

    // Invoke nothing
    event.Invoke();

    std::cout << "Call list size is " << event.CallListSize() << std::endl;

    return 0;
}
```

Possible output:

```c++17
Call list size is 4
Lambda
Function
Static Method
Method
Call list size is 0
```
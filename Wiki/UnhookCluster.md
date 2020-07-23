# UnhookCluster
#### Event<FunctionSignature, KeepOrder, Allocator>::___UnhookCluster___

-----

__void UnhookCluster(EVENT_HANDLE handle);__

Unhooks a cluster of functions hooked by [HookFunctionCluster](https://github.com/itstristanb/Events/wiki/HookFunctionCluster) and [HookMethodCluster](https://github.com/itstristanb/Events/wiki/HookMethodCluster)

##### Parameters
__`handle`__ - Event handle corresponding to the cluster of functions or methods returned by one of the `Hook*Cluster` method

##### Return value
(none)

##### Complexity
O(N) where N is the number of functions or methods hooked by one of the `Hook*Cluster` method

##### Example
```c++
#include "Events.hpp"
#include <iostream>

struct object
{
    void method1()
    {
        std::cout << "Method 1" << std::endl;
    }

    void method2()
    {
        std::cout << "Method 2" << std::endl;
    }
};

void function1()
{
    std::cout << "Function 1" << std::endl;
}

void function2()
{
    std::cout << "Function 2" << std::endl;
}

int main(void)
{
    // Create
    Event<void(void)> event;
    object obj;

    // Hook
    EVENT_HANDLE handle_m = event.HookMethodCluster(obj, &object::method1, &object::method2);
    EVENT_HANDLE handle_f = event.HookFunctionCluster(function1, function2,
                                                      [](){ std::cout << "Lambda 1" << std::endl; },
                                                      [](){ std::cout << "Lambda 2" << std::endl; });

    std::cout << "Size of call list is " << event.CallListSize() << std::endl;

    // Invoke
    event.Invoke();

    // Unhook
    event.UnhookCluster(handle_m);
    event.UnhookCluster(handle_f);

    std::cout << "Size of call list is " << event.CallListSize() << std::endl;

    return 0;
}
```

Possible output:

```c++17
Size of call list is 6
Method 1
Method 2
Function 1
Function 2
Lambda 1
Lambda 2
Size of call list is 0
```
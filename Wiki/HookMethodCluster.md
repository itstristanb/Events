# HookMethodCluster
#### Event<FunctionSignature, KeepOrder, Allocator>::___HookMethodCluster___

-----

__template<typename C, typename ...Fns>  
  [ [ nodiscard \] \] EVENT_HANDLE HookMethodCluster(C &class_ref, Fns... func_ptrs);__

Hooks N number of methods to the call list

##### Parameters
__`class_ref`__ - Reference to the class that has non-static member functions 'func_ptrs'

__`func_ptrs`__ - List of addresses to non-static member functions to be hooked

##### Return value
An EVENT_HANDLE to the corresponding hooked non-static member functions

##### Complexity
O(N) where N is the number of non-static member functions being hooked

##### Notes
Non-static member functions hooked by this function must be unhooked using [UnhookCluster](https://github.com/BeOurQuest/Events/wiki/UnhookCluster).  
Invoke order dependent on template variable __`KeepOrder`__.

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

int main(void)
{
    // Create
    Event<void(void)> event;
    object obj;

    // Hook
    EVENT_HANDLE handle = event.HookMethodCluster(obj,
                                                  &object::method1,
                                                  &object::method2);

    // Invoke
    event.Invoke();

    // Unhook
    event.Unhook(handle);

    return 0;
}
```

Possible output:

```c++17
Method 1
Method 2
```
# HookFunctionCluster
#### Event<FunctionSignature, KeepOrder, Allocator>::___HookFunctionCluster___

-----

__template<typename ...Fns>
  [ [ nodiscard \] \] EVENT_HANDLE HookFunctionCluster(Fns&&... func_ptrs);__

Hooks N number of functions to the call list

##### Parameters
__`func_ptrs`__ - Addresses of lambdas and or functions to be added to the call list

##### Return value
An EVENT_HANDLE to the corresponding hooked functions

##### Complexity
O(N) where N is the number of lambdas and or functions being hooked

##### Notes
Functions hooked by this function must be unhooked using [UnhookCluster](https://github.com/itstristanb/Events/wiki/UnhookCluster).  
Invoke order dependent on template variable __`KeepOrder`__.

##### Example
```c++
#include "Events.hpp"
#include <iostream>

void function1() { std::cout << "function1" << std::endl; }
void function2() { std::cout << "function2" << std::endl; }

int main(void)
{
    // Create
    Event<void(void)> event;

    // Hook
    EVENT_HANDLE handle = event.HookFunctionCluster(
                                                    function1, 
                                                    function2, 
                                                    [](){ std::cout << "lambda1" << std::endl; }, 
                                                    [](){ std::cout << "lambda2" << std::endl; }
                                                   );

    // Invoke
    event.Invoke();

    // Unhook
    event.UnhookCluster(handle);

    return 0;
}
```

Possible output:

```c++17
function1
function2
lambda1
lambda2
```
# UnhookFunctions
#### Event<FunctionSignature, KeepOrder, Allocator>::___UnhookFunctions___

-----

__template\<typename ...Fns\>  
  void UnhookFunctions(Fns ...func_ptrs);__

Removes multiple functions from the call list

##### Parameters
__`func_ptrs`__ - Address of the functions to unhook

##### Return value
(none)

##### Complexity
O(N) where N is the size of the call list

##### Notes
Does not work to unhook lambdas, even if the address has been kept.

```c++
#include "Events.hpp"
#include <iostream>

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

    // Hook
    event.Hook(function1);
    event.Hook(function2);

    std::cout << "Size of call list is " << event.CallListSize() << std::endl;

    // Invoke
    event.Invoke();

    // Unhook
    event.UnhookFunctions(function1, function2);

    std::cout << "Size of call list is " << event.CallListSize() << std::endl;

    return 0;
}
```

Possible output:

```c++17
Size of call list is 2
Function 1
Function 2
Size of call list is 0
```
# CallListSize
#### Event<FunctionSignature, KeepOrder, Allocator>::___CallListSize___

-----

__[ [ nodiscard \] \] size_t CallListSize() const;__

Gets the current size of the call list

##### Parameters
(none)

##### Return value
Size of the call list

##### Complexity
O(1)

##### Example
```c++
#include "Events.hpp"
#include <iostream>

int main(void)
{
    // Create
    Event<void(void)> event;

    std::cout << "Call list size is " << event.CallListSize() << std::endl;

    // Hook
    EVENT_HANDLE handle_1 = event.Hook([](){});
    EVENT_HANDLE handle_2 = event.Hook([](){});

    std::cout << "Call list size is " << event.CallListSize() << std::endl;

    // Unhook
    event.Unhook(handle_1);

    std::cout << "Call list size is " << event.CallListSize() << std::endl;

    // Unhook
    event.Unhook(handle_2);

    std::cout << "Call list size is " << event.CallListSize() << std::endl;

    return 0;
}
```

Possible output:

```c++17
Call list size is 0
Call list size is 2
Call list size is 1
Call list size is 0
```
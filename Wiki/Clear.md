# Clear
#### Event<FunctionSignature, KeepOrder, Allocator>::___Clear___

-----

__void Clear();__

Clears the call list

##### Parameters
(none)

##### Return value
(none)

##### Complexity
O(N) where N is the size of the call list

##### Example
```c++
#include "Events.hpp"
#include <iostream>

struct object
{
    void method(int val)
    {
        std::cout << "Value for method is " << val << std::endl;
    }
};

void function(int val)
{
    std::cout << "Value for function is " << val << std::endl;
}

int main(void)
{
    // Create
    Event<void(int)> event;
    object obj;

    // Hook
    event.Hook(obj, &object::method);
    event.Hook(function);
    EVENT_HANDLE handle = event.Hook([](int val) {
        std::cout << "Value for lambda is " << val << std::endl;
    });

    std::cout << "Call list size is " << event.CallListSize() << std::endl;

    // Invoke
    event.Invoke(123);

    // Clear
    event.Clear();

    std::cout << "Call list size is " << event.CallListSize() << std::endl;

    return 0;
}

```

Possible output:

```c++17
Call list size is 3
Value for method is 123
Value for function is 123
Value for lambda is 123
Call list size is 0
```
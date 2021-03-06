# Invoke
#### Event<FunctionSignature, KeepOrder, Allocator>::___Invoke___

-----

__template<typename ...Args>   
  void Invoke(Args... args);__

Invokes all methods and functions hooked to the call list  

##### Parameters
__`args`__ - Arguments to be passed to each element of the call list

##### Return value
(none)

##### Complexity
O(N) where N is the size of the call list

##### Notes
Order is only guaranteed when the 'KeepOrder' template variable is true.

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

    // Invoke
    event.Invoke(123);


    // Unhook
    event.UnhookClass(obj);
    event.Unhook(handle);
    event.Unhook(function);
    
    return 0;
}
```

Possible output:

```c++17
Value for method is 123
Value for function is 123
Value for lambda is 123
```
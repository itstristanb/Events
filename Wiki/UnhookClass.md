# UnhookClass
#### Event<FunctionSignature, KeepOrder, Allocator>::___UnhookClass___

-----

__template\<typename C\>
  void UnhookClass(C &class_ref);__

Unhooks all methods from the object __`class_ref`__ that have been hooked by this event

##### Parameters
__`class_ref`__ - Reference to the class that has methods hooked to the event

##### Return value
(none)

##### Complexity
O(N) where N is the size of the call list at the time of unhooking

##### Notes
This is the preferred way to unhook all methods from a class.

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
    event.Hook(obj, &object::method1);
    event.Hook(obj, &object::method2);

    std::cout << "Size of call list is " << event.CallListSize() << std::endl;

    // Invoke
    event.Invoke();

    // Unhook
    event.UnhookClass(obj);

    std::cout << "Size of call list is " << event.CallListSize() << std::endl;

    return 0;
}
```

Possible output:

```c++17
Size of call list is 2
Method 1
Method 2
Size of call list is 0
```
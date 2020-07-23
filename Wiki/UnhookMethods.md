# UnhookMethods
#### Event<FunctionSignature, KeepOrder, Allocator>::___UnhookMethods___

-----

__template\<typename C, typename ...Fns\>  
  void UnhookMethods(C &class_ref, Fns ...func_ptrs);__

Removes multiple functions from the call list

##### Parameters
__`class_ref`__ - Reference to the object containing the methods to unhook  

__`func_ptrs`__ - Address of the methods contained within the class `class_ref` to unhook

##### Return value
(none)

##### Complexity
O(N) where N is the size of the call list

##### Notes
If all methods need to be unhooked, consider [UnhookClass](https://github.com/itstristanb/Events/wiki/UnhookClass)

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
    event.UnhookMethods(obj, &object::method1, &object::method2);

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
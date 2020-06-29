# Events
__`Defined in <Events.hpp>`__  
__template<  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; typename FunctionSignature,   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; bool KeepOrder = true,  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; typename Allacator = std::allocator<Call>  
 \> class Event;__

A quick and easy way to modulate reactions to events within a program.

#### Template parameters
__`FunctionSignature`__ - Function signature to invoke. Must be of the form void(types0, type1, ..., typeN) with zero or more arguments.

__`KeepOrder`__ - Determines if the functions are invoked in the order they are hooked. Boost in performance if false. Defaulted as true.

__`Allocator`__ - Allocator for the call list, used in hooking and unhooking functions. Must take type 'Call\<FunctionSignature\>'. Internal mutex map not effected.

#### Member types
|Member type|Definition|
|-----------|------------|
|_Signature|FunctionSignature|
|_Allocator|Allocator|

#### Member functions

##### Interface
|||
|---------|---|
|[Invoke](https://github.com/BeOurQuest/Events/wiki/Invoke)| Goes through the call list, invoking each function <br>___(public member function)___|

|Capacity||
|-------|---|
|[CallListSize](https://github.com/BeOurQuest/Events/wiki/CallListSize)|Gets the size of the call list <br>___(public member function)___|

##### Modifiers
|||
|---------|---|
|[(Destructor)](https://github.com/BeOurQuest/Events/wiki/Destructor)|Clears the call list and removed itself from the mutex map <br>___(public member function)___|
|[Hook](https://github.com/BeOurQuest/Events/wiki/Hook)|Hooks a method or function to the call list <br>___(public member function)___|
|[HookFunctionCluster](https://github.com/BeOurQuest/Events/wiki/HookFunctionCluster)|Hooks multiple functions to the call list <br>___(public member function)___|
|[HookMethodCluster](https://github.com/BeOurQuest/Events/wiki/HookMethodCluster)|Hooks multiple methods to the call list <br>___(public member function)___|
|[Unhook](https://github.com/BeOurQuest/Events/wiki/Unhook)|Unhooks a function from the call list ___(public member function)___|
|[UnhookCluster](https://github.com/BeOurQuest/Events/wiki/UnhookCluster)|Unhooks a cluster functions from the call list hooked by one of the 'Cluster' member functions <br>___(public member function)___|
|[UnhookClass](https://github.com/BeOurQuest/Events/wiki/UnhookClass)|Unhooks all methods from the call list of the class hooked <br>___(public member function)___|
|[UnhookFunctions](https://github.com/BeOurQuest/Events/wiki/UnhookFunctions)|Unhooks multiple functions from the call list <br>___(public member function)___|
|[UnhookMethods](https://github.com/BeOurQuest/Events/wiki/UnhookMethods)|Unhooks multiple methods from the call list <br>___(public member function)___|
|[Clear](https://github.com/BeOurQuest/Events/wiki/Clear)|Clears all methods and functions from the call list <br>___(public member function)___|

##### Helper class'
|||
|-------------|---|
|[Call](https://github.com/BeOurQuest/Events/wiki/Call)|Container for method or function in call list <br>___(private class definition)___|
|[CallHash](https://github.com/BeOurQuest/Events/wiki/CallHash)|Hashing policy class for 'Call' type <br>___(private class definition)___|
|[USet](https://github.com/BeOurQuest/Events/wiki/USet)|Wrapper around std::unordered_set to standardize the 'emplace_back' method <br>___(private class definition)___|
|[is_member_function_of](https://github.com/BeOurQuest/Events/wiki/is_member_function_of)|Contains member variable checking if a method is contained within a class <br>___(private class definition)___|
|[parameter_equivalents](https://github.com/BeOurQuest/Events/wiki/parameter_equivalents)|Contains member variable checking if a list of arguments is the same as the Event's function signature arguments <br>___(private class definition)___|

##### Helper functions
|||
|----------------|---|
|[RemoveCluster](https://github.com/BeOurQuest/Events/wiki/RemoveCluster)|Removes a cluster by EVENT_HANDLE <br>___(private static member function)___|
|[RemoveCall](https://github.com/BeOurQuest/Events/wiki/RemoveCall)|Removes a method or function from call list <br>___(private static member function)___|
|[POINTER_INT_CAST](https://github.com/BeOurQuest/Events/wiki/POINTER_INT_CAST)|Converts a pointer into its int value <br>___(private static member function)___|
|[invocable](https://github.com/BeOurQuest/Events/wiki/invocable)|Returns true if the Event is invokable <br>___(private static member function)___|
|[type_exclusion](https://github.com/BeOurQuest/Events/wiki/type_exclusion)|Returns true if a type is not included within a list <br>___(private static member function)___|
|[class_member_inclusion](https://github.com/BeOurQuest/Events/wiki/class_member_inclusion)|Returns true if a method is contained within a class <br>___(private static member function)___|
|[class_member_exclusion](https://github.com/BeOurQuest/Events/wiki/class_member_exclusion)|Returns true if a function list doesn't contain a member function <br>___(private static member function)___|
|[is_same_arg_list](https://github.com/BeOurQuest/Events/wiki/is_same_arg_list)|Returns true if two argument lists are the same <br>___(private static member function)___|

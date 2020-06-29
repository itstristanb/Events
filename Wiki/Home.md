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
|[Invoke](https://github.com/BeOurQuest/Events/wiki)| Goes through the call list, invoking each function <br>___(public member function)___|

|Capacity||
|-------|---|
|[CallListSize](https://github.com/BeOurQuest/Events/wiki)|Gets the size of the call list <br>___(public member function)___|

##### Modifiers
|||
|---------|---|
|[(Destructor)](https://github.com/BeOurQuest/Events/wiki)|Clears the call list and removed itself from the mutex map <br>___(public member function)___|
|[Hook](https://github.com/BeOurQuest/Events/wiki)|Hooks a method or function to the call list <br>___(public member function)___|
|[HookFunctionCluster](https://github.com/BeOurQuest/Events/wiki)|Hooks multiple functions to the call list <br>___(public member function)___|
|[HookMethodCluster](https://github.com/BeOurQuest/Events/wiki)|Hooks multiple methods to the call list <br>___(public member function)___|
|[Unhook](https://github.com/BeOurQuest/Events/wiki)|Unhooks a function from the call list ___(public member function)___|
|[UnhookCluster](https://github.com/BeOurQuest/Events/wiki)|Unhooks a cluster functions from the call list hooked by one of the 'Cluster' member functions <br>___(public member function)___|
|[UnhookClass](https://github.com/BeOurQuest/Events/wiki)|Unhooks all methods from the call list of the class hooked <br>___(public member function)___|
|[UnhookFunctions](https://github.com/BeOurQuest/Events/wiki)|Unhooks multiple functions from the call list <br>___(public member function)___|
|[UnhookMethods](https://github.com/BeOurQuest/Events/wiki)|Unhooks multiple methods from the call list <br>___(public member function)___|
|[Clear](https://github.com/BeOurQuest/Events/wiki)|Clears all methods and functions from the call list <br>___(public member function)___|

##### Helper class'
|||
|-------------|---|
|[Call](https://github.com/BeOurQuest/Events/wiki)|Container for method or function in call list <br>___(private class definition)___|
|[CallHash](https://github.com/BeOurQuest/Events/wiki)|Hashing policy class for 'Call' type <br>___(private class definition)___|
|[USet](https://github.com/BeOurQuest/Events/wiki)|Wrapper around std::unordered_set to standardize the 'emplace_back' method <br>___(private class definition)___|
|[is_member_function_of](https://github.com/BeOurQuest/Events/wiki)|Contains member variable checking if a method is contained within a class <br>___(private class definition)___|
|[parameter_equivalents](https://github.com/BeOurQuest/Events/wiki)|Contains member variable checking if a list of arguments is the same as the Event's function signature arguments <br>___(private class definition)___|

##### Helper functions
|||
|----------------|---|
|[RemoveCluster](https://github.com/BeOurQuest/Events/wiki)|Removes a cluster by EVENT_HANDLE <br>___(private static member function)___|
|[RemoveCall](https://github.com/BeOurQuest/Events/wiki)|Removes a method or function from call list <br>___(private static member function)___|
|[POINTER_INT_CAST](https://github.com/BeOurQuest/Events/wiki)|Converts a pointer into its int value <br>___(private static member function)___|
|[invocable](https://github.com/BeOurQuest/Events/wiki)|Returns true if the Event is invokable <br>___(private static member function)___|
|[type_exclusion](https://github.com/BeOurQuest/Events/wiki)|Returns true if a type is not included within a list <br>___(private static member function)___|
|[class_member_inclusion](https://github.com/BeOurQuest/Events/wiki)|Returns true if a method is contained within a class <br>___(private static member function)___|
|[class_member_exclusion](https://github.com/BeOurQuest/Events/wiki)|Returns true if a function list doesn't contain a member function <br>___(private static member function)___|
|[is_same_arg_list](https://github.com/BeOurQuest/Events/wiki)|Returns true if two argument lists are the same <br>___(private static member function)___|

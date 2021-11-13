/*!
 * \author Tristan Florian Bouchard
 * \file   Events.hpp
 * \data   4/20/2020
 * \brief  This file contains the implementation for a templated C++ Event system. See README.md for more info
 * \par    link: https://github.com/BeOurQuest/Events.git
 */

//  /$$$$$$$$ /$$    /$$ /$$$$$$$$ /$$   /$$ /$$$$$$$$  /$$$$$$       /$$   /$$ /$$$$$$$  /$$$$$$$
// | $$_____/| $$   | $$| $$_____/| $$$ | $$|__  $$__/ /$$__  $$     | $$  | $$| $$__  $$| $$__  $$
// | $$      | $$   | $$| $$      | $$$$| $$   | $$   | $$  \__/     | $$  | $$| $$  \ $$| $$  \ $$
// | $$$$$   |  $$ / $$/| $$$$$   | $$ $$ $$   | $$   |  $$$$$$      | $$$$$$$$| $$$$$$$/| $$$$$$$/
// | $$__/    \  $$ $$/ | $$__/   | $$  $$$$   | $$    \____  $$     | $$__  $$| $$____/ | $$____/
// | $$        \  $$$/  | $$      | $$\  $$$   | $$    /$$  \ $$     | $$  | $$| $$      | $$
// | $$$$$$$$   \  $/   | $$$$$$$$| $$ \  $$   | $$   |  $$$$$$/ /$$ | $$  | $$| $$      | $$
// |________/    \_/    |________/|__/  \__/   |__/    \______/ |__/ |__/  |__/|__/      |__/

// Copyright (c) 2020-present, Tristan Florian Bouchard
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef EVENTS_HPP
#define EVENTS_HPP
#pragma once
#pragma warning(push)
#pragma warning(disable : 4003)

#include <unordered_set> // unordered_set
#include <functional>    // function
#include <cassert>       // assert
#include <cstdint>       // uint64_t
#include <vector>        // vector
#include <unordered_map> // unordered_map

// TODO: Assert when unhooking an event that is not found
// TODO: Static Assert when hooking or unhooking a class pointer in the hook or unhook method
// TODO: Support std::function/anything callable
// TODO: Ensure no data loss for handles
// TODO: Bind pointers to the event (must be class pointer if not null)
// TODO: Delete copy constructor/move constructor
// TODO: Add assignment operation that replaces the this pointer calls
// TODO [maybe]: Remove clusters?
// TODO: Wrap things like the call in an internal namespace
// TODO: Reformat comments and make sure they are up to date
// TODO: Have it so that the call stores raw pointers to the class and member function, and split the signature into the return value and arguments
// TODO: Maybe add it where you can update the bounded function

//! For type checking with a cleaner syntax
#define VERIFY_TYPE noexcept

//! Generates decorators for noexcept
#define PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_REF_OPT) \
      MACRO(CV_REF_OPT); \
      MACRO(CV_REF_OPT noexcept)

//! Generates decorators for cv ref
#define PERMUTE_PMF_CV_REF(MACRO, CV_OPT) \
      PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT); \
      PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT &); \
      PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT &&)

//! Generates decorator for cv
#define PERMUTE_PMF_CV(MACRO) \
      PERMUTE_PMF_CV_REF(MACRO, ); \
      PERMUTE_PMF_CV_REF(MACRO, const); \
      PERMUTE_PMF_CV_REF(MACRO, volatile); \
      PERMUTE_PMF_CV_REF(MACRO, const volatile)

//! Generates the above macros to cover all decorator
#define PERMUTE_PMF(MACRO) \
      PERMUTE_PMF_CV(MACRO); \
      PERMUTE_PMF_CV(MACRO##_ELLIPSIS)

/*!
 * \brief
 *   Acts as a way to identify what and how a 'Call' was hooked to an event
 */
struct EventHandle
{
    // TODO: Maybe store the actual pointers for the call to use?
    EventHandle() = default;
    EventHandle(const EventHandle& other) = default;

    template<typename I, typename F>
    EventHandle(uint16_t priority, I identifier, F function)
            : priority(priority),
              identifier(POINTER_INT_CAST(identifier)),
              function(POINTER_INT_CAST(function))
    {
    }

    bool operator<(const EventHandle& other) const
    {
        return (priority < other.priority) &&
               (identifier < other.identifier) &&
               (function < other.function);
    }

    bool operator==(const EventHandle& other) const
    {
        return (priority == other.priority) &&
               (identifier == other.identifier) &&
               (function == other.function);
    }

    [[nodiscard]] inline uint16_t GetPriority() const
    {
        return priority;
    }

    [[nodiscard]] inline uint64_t GetIdentifier() const
    {
        return identifier;
    }

    [[nodiscard]] inline uint64_t GetFunction() const
    {
        return function;
    }

    static const uint64_t FunctionIdentifier = 1;
    static const uint64_t LambdaIdentifier = 2;
    static const uint64_t StdFunctionIdentifier = 3;
private:

    /*!
     * \brief
     *   Mapping:
     *                                           |  priority  |  identifier  |     function     |
     *   if non-member function        = | [0, 65535] | FunctionIdentifier    | &function        |
     *   if lambda function            = | [0, 65535] | LambdaIdentifier      | &lambda          |
     *   if std::function              = | [0, 65535] | StdFunctionIdentifier | &std::function   |
     *   if non-static member function = | [0, 65535] | &class                | &member_function |
     */
    uint16_t priority = 0;   //!< Function call's priority in the event
    uint64_t identifier = 0; //!< Either a unique value or this pointer of member function
    uint64_t function = 0;   //!< Address of the function (member or other)

    /*!
     * \brief
     *   Converts a pointer address into the int literal of it
     * \param t
     *   The pointer to convert
     * \return
     *   The pointer's address as an uint64_t
     */
    template<typename T>
    static constexpr uint64_t POINTER_INT_CAST(T t)
    {
        return reinterpret_cast<uint64_t>(*reinterpret_cast<void**>(&t));
    }
};

namespace Internal
{
    /*!
     * \brief
     *   Callback wrapper
     */
    template<typename Signature> struct Call {};
    template<typename R, typename ...Args>
    struct Call<R(Args...)>
    {
        /*!
         * \brief
         *   Constructor for std::functions
         */
         Call(std::function<R(Args...)> func, EventHandle handle)
            :  handle(handle),
               function([f = std::move(func)](void*, Args ...args) mutable {
                   f(args...);
               })
        {
        }

        /*!
         * \brief
         *   Constructor for non-member function and lambda'
         */
        template<typename Fn>
        Call(Fn func_ptr, EventHandle handle)
                : handle(handle),
                  function([&func_ptr](void*, Args ...args) mutable {
                      func_ptr(args...);
                  })
        {
        }

        /*!
         * \brief
         *   Constructor for non-static member function
         * \param class_ptr
         *   Pointer to class
         * \param func_ptr
         *   Pointer to member function contained within class C
         * \param handle
         *   Handle corresponding to member function
         */
        template<typename C, typename Fn>
        Call(C* class_ptr, Fn func_ptr, EventHandle handle)
                : handle(handle),
                  class_ptr(class_ptr),
                  function([func_ptr](void *class_addr, Args... args) mutable {
                      (void)(((C*)(class_addr))->*func_ptr)(args...);
                  })
        {
        }

        void operator()(Args ...args) const
        {
            function(class_ptr, args...);
        }

        bool operator==(const Call<R(Args...)>& other) const
        {
            return handle == other.handle;
        }

        bool operator==(const EventHandle& other) const
        {
            return handle == other;
        }

        explicit operator EventHandle() const
        {
            return handle;
        }

        mutable EventHandle handle;         //!< Handle corresponding to the function
        mutable void* class_ptr = nullptr;  //!< A pointer to the class if the non-static member function constructor was used
    private:
        std::function<void(void*)> function; //!< Function to call
    };
}

/*!
 * \brief
 *   Templated event system that holds clients callbacks to be
 *   invoked on elsewhere
 * \tparam FunctionSignature
 *   Function signature of the callbacks to hold
 *   IMPORTANT: All callbacks must contain the same argument types as this Signature's and
 *   the return value will be dropped to void if not an std::function
 * \tparam KeepOrder
 *   Tells the system to invoke callbacks in the same order as they
 *   were hooked. That is, changing the priority when KeepOrder is true is a no-op
 * \tparam Allocator
 *      Allocator for the call list. Must take struct 'Call'
 */
template<typename FunctionSignature, bool KeepOrder = false, typename Allocator = std::allocator<Internal::Call<FunctionSignature>>>
class Event
{
    /*!
     * \brief
     *   Ensures that 'Fn' is a non-member function and is not an object
     * \tparam Fn
     *   Type to check
     */
    template<typename Fn>
    static constexpr bool class_exclusion()
    {
        static_assert(!std::is_class_v<Fn>, "Attempting to use class pointer as function pointer");
        static_assert(!std::is_member_function_pointer_v<Fn>, "Is a class member function when it should not be");
        return true;
    }

    /*!
     * \brief
     *   Ensures this event can be invoked by arguments 'Args'
     * \tparam Args
     *   List of type of arguments attempted to invoke the event with
     */
    template<typename ...Args>
    static constexpr bool invocable()
    {
        static_assert(std::is_invocable_v<_Signature, Args...>, "Attempting to invoke event with differing arguments then the event function signature");
        return true;
    }

    /*!
     * \brief
     *   Ensures 'C' is an object type and 'Fn' is a non-static member function within 'C'
     * \tparam C
     *   Type of object Fn must be apart of
     * \tparam Fn
     *   Function to be contained within 'C'
     */
    template<typename C, typename Fn>
    static constexpr bool class_inclusion()
    {
        static_assert(std::is_class_v<C>, "Provided class pointer C is not a class");
        static_assert(is_member_function_of<C, Fn>::value, "Fn is not a non-static member of object C");
        return true;
    }

    /*!
     * \brief
     *   Checks if the parameter list of 'Fn' matches that of 'FunctionSignature''s
     * \tparam Fn
     *   Function to check parameter list of
     */
    template<typename Fn>
    static constexpr bool is_same_arg_list()
    {
        static_assert(parameter_equivalents<_Signature, Fn>::value, "Attempted to hook a callback that does not have the same parameter list as the event");
        return true;
    }
public:
    using _Signature = FunctionSignature;                //!< Function Signature
    using _Allocator = Allocator;                        //!< Event allocator
    using _CallType = Internal::Call<FunctionSignature>; //!< Type of the call wrapper
    static constexpr bool Ordered = KeepOrder;           //!< State of ordering
    Event(const Event& other) = delete;
    Event(Event&& other) = delete;
    ~Event() = default;

    /*!
     * \brief
     *   If this event is not static and contained within an object, you MUST pass in it's this pointer to the event constructor
     * \param bindTo
     *   Must be nullptr
     */
    Event(std::nullptr_t bindTo)
            : boundTo(bindTo)
    {
    }

    /*!
     * \brief
     *   If this event is contained within an object, you MUST pass in it's this pointer to the constructor
     *   IMPORTANT: Assuming this event is not static
     * \param bindTo
     *   The this pointer to the object this function is apart of
     */
    template<typename Bind>
    Event(Bind* bindTo)
            : boundTo(bindTo)
    {
        static_assert(std::is_class_v<Bind>, "Bind must be a pointer to the object it is bound to");
    }

    /*!
     * \brief
     *   Copies over event callbacks
     * \param other
     *   The event to copy
     */
    Event& operator=(const Event& other)
    {
        if (&other == this)
            return *this;
        callList_ = other.callList_;
        priority_ = other.priority_;
        if (boundTo != nullptr && other.boundTo != nullptr)
            ReplaceBound(other.boundTo);
        return *this;
    }

    /*!
     * \brief
     *   Hooks a std::function to the event provided that the signature of 'func'
     *   matches that of 'FunctionSignature'
     * \tparam PRIORITY
     *   Priority given to the function
     *   IMPORTANT: guarantees to be called before functions with a higher priority value,
     *   but has an undefined
     * \param func
     *   The std::function to hook
     * \return
     *   Returns a handle corresponding to the hooked std::function.
     *   IMPORTANT: Must not be ignored when hooking, otherwise it become permanently hooked
     */
    template<uint16_t PRIORITY = 0>
    [nodiscard] EventHandle Hook(std::function<_Signature> func)
    {
        uint16_t priority = Ordered ? priority_++ : PRIORITY;
        EventHandle handle(priority, EventHandle::StdFunctionIdentifier, func);
        callList_[priority].emplace_back(Internal::Call<_Signature>(func, handle));
        return handle;
    }

    /*!
     * \brief
     *   Hooks a lambda function to the event provided that the argument types of 'lambda'
     *   matches that of 'FunctionSignature''s
     * \tparam PRIORITY
     *   Priority given to the function
     *   IMPORTANT: guarantees to be called before functions with a higher priority value,
     *   but has an undefined
     * \param lambda
     *   The lambda to hook
     * \return
     *   Returns a handle corresponding to the hooked lambda.
     *   IMPORTANT: Must not be ignored when hooking, otherwise it become permanently hooked
     */
    template<uint16_t PRIORITY = 0, typename Fn>
    [nodiscard] EventHandle Hook(Fn&& lambda)
    VERIFY_TYPE(is_same_arg_list<Fn>())
    {
        uint16_t priority = Ordered ? priority_++ : PRIORITY;
        EventHandle handle(priority, EventHandle::LambdaIdentifier, &lambda);
        callList_[priority].emplace_back(Internal::Call<_Signature>(lambda, handle));
        return handle;
    }

    /*!
     * \brief
     *   Hooks a non-member function to the event provided that the argument types of 'func_ptr'
     *   matches that of 'FunctionSignature''s
     *   IMPORTANT: If you hook the same function with differing priorities, you must specify
     *   the priority, or use the handle when Unhooking it
     * \tparam PRIORITY
     *   Priority given to the function
     *   IMPORTANT: guarantees to be called before functions with a higher priority value,
     *   but has an undefined
     * \param func_ptr
     *   Pointer to non-member function to be hooked
     * \return
     *   Returns a handle corresponding to the hooked function.
     */
    template<uint16_t PRIORITY = 0, typename Fn>
    EventHandle Hook(Fn* func_ptr)
    VERIFY_TYPE(class_exclusion<Fn>() && is_same_arg_list<Fn>())
    {
        uint16_t priority = Ordered ? priority_++ : PRIORITY;
        EventHandle handle(priority, EventHandle::FunctionIdentifier, func_ptr);
        callList_[priority].emplace_back(Internal::Call<_Signature>(func_ptr, handle));
        return handle;
    }

    /*!
     * \brief
     *   Hooks a non-static member function to the event system
     *   IMPORTANT: If you hook the same non-static member function with differing priorities,
     *   you must specify the priority, or use the handle when Unhooking it.
     *   It is best practice to use UnhookClass to remove class-hooked calls
     * \tparam PRIORITY
     *   Priority given to the function
     *   IMPORTANT: guarantees to be called before functions with a higher priority value,
     *   but has an undefined
     * \param class_ref
     *   Reference to the class that has non-static member function 'func_ptr'
     * \param func_ptr
     *   Pointer to non-static member function to hook
     * \return
     *   Returns handle corresponding to non-static member function hooked
     */
    template<uint16_t PRIORITY = 0, typename C, typename Fn>
    EventHandle Hook(C* class_ptr, Fn func_ptr)
    VERIFY_TYPE(class_inclusion<C, Fn>() && is_same_arg_list<Fn>())
    {
        uint16_t priority = Ordered ? priority_++ : PRIORITY;
        EventHandle handle(priority, class_ptr, func_ptr);
        callList_[priority].emplace_back(Internal::Call<_Signature>(class_ptr, func_ptr, handle));
        return handle;
    }

    /*!
     * \brief
     *   Invokes callbacks hooked to the event
     *   IMPORTANT: Hooking or Unhooking to the same event during the invoke process is undefined
     *   IMPORTANT: Order that the callbacks get invoked is undefined unless specified with a priority
     * \param args
     *   Parameters to pass to each of the callback functions
     *   IMPORTANT: Must be the same arguments as the FUNCTION_SIGNATURE
     */
    template<typename ...Args>
    void Invoke(Args... args)
    VERIFY_TYPE(invocable<Args...>())
    {
        for (auto& priority : callList_)
            for (auto& call : priority.second)
                (call)(args...);
    }

    /*!
     * \brief
     *   Unhooks non-member function from event
     * \param func_ptr
     *   Pointer to non-member function to unhook
     */
    template<uint16_t PRIORITY = 0, typename Fn>
    void Unhook(Fn* func_ptr)
    VERIFY_TYPE(class_exclusion<Fn>())
    {
        RemoveCall(EventHandle(PRIORITY, EventHandle::FunctionIdentifier, func_ptr));
    }

    /*!
     * \brief
     *   Unhooks non-static member function from event
     * \param class_ptr
     *   Address of the user defined type that contains the non-static member function 'func_ptr'
     * \param func_ptr
     *   Pointer to non-static member function to unhook
     */
    template<uint16_t PRIORITY = 0, typename C, typename Fn>
    void Unhook(C* class_ptr, Fn func_ptr)
    VERIFY_TYPE(class_inclusion<C, Fn>())
    {
        RemoveCall(EventHandle(PRIORITY, class_ptr, func_ptr));
    }

    /*!
     * \brief
     *   Unhooks a function that a handle corresponds to
     * \param handle
     *   Handle corresponding to the function to unhook
     */
    void Unhook(EventHandle handle)
    {
        RemoveCall(handle);
    }

    // TODO: See you can call the raw *
    // TODO: Maybe just use void pointers for the event handle?

    /*!
     * \brief
     *   Unhooks all non-static member functions hooked with the user defined type
     * \param class_ptr
     *   Address of the class that has non-static member functions hooked to event
     */
    template<typename C>
    void UnhookClass(C* class_ptr)
    {
        static_assert(std::is_class_v<C>, "class_ptr must be a pointer to a class");
        bool unhooked = false;
        EventHandle handle(0, class_ptr, 0);
        for (auto& call_list : callList_)
            for (auto it = call_list.second.begin(); it != call_list.second.end();)
                if (it->handle.GetIdentifier() == handle.GetIdentifier())
                {
                    unhooked = true;
                    it = call_list.second.erase(it);
                }
                else
                    ++it;
        assert(unhooked && "No member functions were found to be hooked by the class. Check that the member functions aren't already unhooked");
    }

    /*!
     * \brief
     *      Getter for how many callbacks are stored within this event
     *
     * \return
     *      Returns number of callbacks hooked to this event
     */
    [[nodiscard]] size_t CallListSize() const
    {
        size_t size = 0;
        for (auto& call_list : callList_)
            size += call_list.second.size();
        return size;
    }

    /*!
     * \brief
     *   Clears the call list
     */
    void Clear()
    {
        for (auto& call_list : callList_)
            call_list.second.clear();
    }

private:
    struct USet;
    struct CallHash; // forward declare

    void* boundTo;                                //!< The object or it is bound to
    std::unordered_map<uint16_t, USet> callList_; //!< List of callbacks key=priority value=call_list
    uint16_t priority_ = 0;                       //!< Increases on each Hook* when KeepOrder is true

    /*!
     * \brief
     *      Unhooks a handle from the call list
     *
     * \param handle
     *      Handle to the function to unhook
     */
    void RemoveCall(EventHandle handle)
    {
        auto& call_list = callList_[handle.GetPriority()];
        auto call = std::find(call_list.begin(), call_list.end(), handle);
        assert(call != call_list.end() && "ERROR : Attempting to unhook call count not be found or does not exist. Check the priority or if it was previously unhooked");
        call_list.erase(call);
    }

    void ReplaceBound(void* toReplace)
    {
        for (auto& call_list : callList_)
            for (const Internal::Call<_Signature>& call : call_list.second)
                // If the call is a member function of to the bounded object
                if (call.class_ptr == toReplace)
                {
                    // Update what it points to
                    call.class_ptr = boundTo;
                    call.handle = EventHandle(call.handle.GetPriority(), boundTo, call.handle.GetFunction());
                }
    }

    /*!
     * \brief
     *      Hash functor used in unordered_set
     */
    struct CallHash
    {
        size_t operator()(const Internal::Call<_Signature>& call) const
        {
            // Combine all three properties with XOR
            uint16_t priority = call.handle.GetPriority();
            uint64_t identifier = call.handle.GetIdentifier();
            uint64_t function = call.handle.GetFunction();
            return ((std::hash<uint16_t>()(priority) ^
                    (std::hash<uint64_t>()(identifier) << 1)) >> 1) ^
                    (std::hash<uint64_t>()(function) << 1);
        }
    };

    /*!
     * \brief
     *      Wrapper around an unordered_set to standard the emplace_back function
     */
    struct USet : public std::unordered_set<Internal::Call<_Signature>, CallHash, std::equal_to<Internal::Call<_Signature>>, _Allocator>
    {
        /*!
         * \brief
         *   Wrapper around emplace used for verification
         * \param args
         *   Arguments to the constructor of Call data type
         */
        template<typename ...Args>
        void emplace_back(Args... args)
        {
            bool was_added = this->emplace(args...).second;
            assert(was_added && "ERROR : Duplicate function hooked to event with same priority");
        }
    };

    /*!
     * \brief
     *      Base case, not a member function of a class
     */
    template<typename, typename>
    struct is_member_function_of
    {
        static constexpr bool value = false;
    };

    /*!
     * \brief
     *      Generates all overloads for member functions and their decorators
     */
#define DEF_IS_MEMBER_FUNCTION_OF(CV_REF_NOEXCEPT_OPT) \
      template<typename C, typename R, typename... Args> \
      struct is_member_function_of<C, R(C::*)(Args...) CV_REF_NOEXCEPT_OPT> \
      { static constexpr bool value = true; }
#define DEF_IS_MEMBER_FUNCTION_OF_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
      template<typename C, typename R, typename... Args> \
      struct is_member_function_of<C, R(C::*)(Args..., ...) CV_REF_NOEXCEPT_OPT> \
      { static constexpr bool value = true; }
    PERMUTE_PMF(DEF_IS_MEMBER_FUNCTION_OF);

    /*!
     * \brief
     *      Base case, is a lambda, check if it can be constructed as an std::function
     */
    template<typename COMP, typename Fn>
    struct parameter_equivalents
    {
        static constexpr bool value = std::is_constructible_v<std::function<_Signature>, Fn>;
    };

    /*!
     * \brief
     *     Non-member function overload to see if COMP can be invoked with arguments Args
     */
    template<typename COMP, typename R, typename ...Args>
    struct parameter_equivalents<COMP, R(*)(Args...)>
    {
        static constexpr bool value = std::is_invocable_v<COMP, Args...>;
    };

    /*!
     * \brief
     *      Generates all overloads to check if COMP can be invoked with
     *      the arguments of Args
     */
#define DEF_PARAMETER_EQUIVALENTS(CV_REF_NOEXCEPT_OPT) \
    template<typename COMP, typename C, typename R, typename ...Args> \
    struct parameter_equivalents<COMP, R(C::*)(Args...) CV_REF_NOEXCEPT_OPT> \
    { static constexpr bool value = std::is_invocable_v<COMP, Args...>; };
#define DEF_PARAMETER_EQUIVALENTS_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
    template<typename COMP, typename C, typename R, typename ...Args> \
    struct parameter_equivalents<COMP, R(C::*)(Args..., ...) CV_REF_NOEXCEPT_OPT> \
    { static constexpr bool value = std::is_invocable_v<COMP, Args...>; };
    PERMUTE_PMF(DEF_PARAMETER_EQUIVALENTS);
};

#pragma warning(pop)
#endif

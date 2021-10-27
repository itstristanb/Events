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

#include <unordered_set> // unordered_set
#include <functional>    // function
#include <cassert>       // assert
#include <cstdint>       // uint64_t
#include <vector>        // vector
#include <map>           // map

//! For variadic template expansion
#define PACK_EXPAND(function, ...) ((void)function(__VA_ARGS__), ...);

/*!
 * \brief
 *      Handle to a function within the event call list
 *      Layout of EVENT_HANDLE memory:
 *                                              |  16bits  |   16bits   |       32bits     |
 *      if non-member function                = | Priority | 0          | &function        |
 *      if non-static member function         = | Priority | &class     | &member_function |
 *      if non-member function cluster        = | Priority | unique val | &function        |
 *      if non-static member function cluster = | Priority | unique val | &member_function |
 */
using EVENT_HANDLE = uint64_t;

//! Bit masks for getting the first n bits
#define BITMASK_FIRST_32 0b11111111111111111111111111111111
#define BITMASK_FIRST_16 0b1111111111111111

//! Helper macros to abstract redundant code for handle values
#define EVENT_SET_PROPERTY(__handle, __value, lsb_mask__, field_offset__, clear_mask__) \
(((EVENT_HANDLE(__value) & (lsb_mask__)) << (field_offset__)) | (EVENT_HANDLE(__handle) & (clear_mask__)))
#define EVENT_GET_PROPERTY(__handle, field_offset__, lsb_mask__) \
((EVENT_HANDLE(__handle) >> (field_offset__)) & (lsb_mask__))

//! Priority handle layout utility macros
#define EVENT_PRIORITY_OFFSET 48
#define EVENT_PRIORITY_CLEAR_MASK 0b0000000000000000'1111111111111111'11111111111111111111111111111111
#define SET_PRIORITY(__handle, __value) EVENT_SET_PROPERTY(__handle, __value, BITMASK_FIRST_16, EVENT_PRIORITY_OFFSET, EVENT_PRIORITY_CLEAR_MASK)
#define GET_PRIORITY(__handle) ((EVENT_HANDLE(__handle) >> EVENT_PRIORITY_OFFSET) & BITMASK_FIRST_16)

//! Cluster handle layout utility macros
#define EVENT_CLUSTER_OFFSET 32
#define EVENT_CLUSTER_CLEAR_MASK 0b1111111111111111'0000000000000000'11111111111111111111111111111111
#define SET_CLUSTER(__handle, __value) EVENT_SET_PROPERTY(__handle, __value, BITMASK_FIRST_16, EVENT_CLUSTER_OFFSET, EVENT_CLUSTER_CLEAR_MASK)
#define GET_CLUSTER(__handle) EVENT_GET_PROPERTY(__handle, EVENT_CLUSTER_OFFSET, BITMASK_FIRST_16)

//! Id handle layout utility macros
#define EVENT_ID_OFFSET 0
#define EVENT_ID_CLEAR_MASK 0b1111111111111111'1111111111111111'00000000000000000000000000000000
#define SET_ID(__handle, __value) EVENT_SET_PROPERTY(__handle, __value, BITMASK_FIRST_32, EVENT_ID_OFFSET, EVENT_ID_CLEAR_MASK)
#define GET_ID(__handle) EVENT_GET_PROPERTY(__handle, EVENT_ID_OFFSET, BITMASK_FIRST_32)

//! Constructs a handle with the last 4 bytes has the cluster and the first 4 be the ID
#define MAKE_HANDLE(__priority, __cluster, __id) \
  (                                              \
      SET_PRIORITY(0, __priority) |              \
      SET_CLUSTER(0, __cluster) |                \
      SET_ID(0, __id)                            \
 )

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
 *      Callback wrapper
 */
template<typename Signature>
struct Call
{
    /*!
     * \brief
     *      Default Constructor
     */
    Call() : function([](){}), handle(EVENT_HANDLE(0))
    {}

    /*!
     * \brief
     *      Constructor for non-member function
     *
     * \tparam F
     *      Type of function
     *
     * \param func_ptr
     *      Pointer to function to hook
     *
     * \param handle
     *      Handle corresponding to the function 'func_ptr'
     */
    template<typename Fn>
    Call(Fn func_ptr, EVENT_HANDLE handle) : function(func_ptr), handle(handle)
    {}

    /*!
     * \brief
     *      Constructor for non-static member function
     *
     * \tparam C
     *      Type of class
     *
     * \tparam F
     *      Type of function
     *
     * \param class_ptr
     *      Pointer to class
     *
     * \param func_ptr
     *      Pointer to member function contained within class C
     *
     * \param handle
     *      Handle corresponding to member function
     */
    template<typename C, typename Fn>
    Call(C *class_ptr, Fn func_ptr, EVENT_HANDLE handle) : function(GetMethod(class_ptr, func_ptr)), handle(handle)
    {}

    /*!
     * \brief
     *      Assignment operator that checks if the handles are the same
     *
     * \param other
     *      Other call to check the handle of
     *
     * \return
     *      Returns true if the handles are the same
     */
    bool operator==(const Call<Signature>& other) const
    {
        return handle == other.handle;
    }

    /*!
     * \brief
     *      Assignment operator that checks if the handles are the same
     *
     * \param handle
     *      Handle to compare with this handle
     *
     * \return
     *      Returns true if the handles are the same
     */
    bool operator==(EVENT_HANDLE eventHandle) const
    {
        return handle == eventHandle;
    }

    /*!
     * \brief
     *      Implicit conversion operator
     *
     * \return
     *      Returns handle of call
     */
    explicit operator EVENT_HANDLE()
    {
        return handle;
    }

    /*!
     * \brief
     *      Overload for all non-ellipsis member functions
     *
     */
#define DEF_GET_METHOD(CV_REF_NOEXCEPT_OPT) \
        template<typename C, typename R, typename ...Args> \
        auto GetMethod(C *class_ptr, R(C::*func_ptr)(Args...) CV_REF_NOEXCEPT_OPT) \
        { \
          return [class_ptr, func_ptr] (Args... args) mutable { (void)(std::mem_fn(func_ptr)(class_ptr, args...)); }; \
        }

    /*!
     * \brief
     *      Overload for all ellipsis member functions
     */
#define DEF_GET_METHOD_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
        template<typename C, typename R, typename ...Args> \
        auto GetMethod(C *class_ptr, R(C::*func_ptr)(Args..., ...) CV_REF_NOEXCEPT_OPT) \
        { \
          return [class_ptr, func_ptr](Args... args) mutable { (void)(std::mem_fn(func_ptr)(class_ptr, args...)); }; \
        }

    /*!
     * \brief
     *      Generates all overloaded member functions for GetMethod
     *
     * \tparam C
     *      Type of class
     *
     * \tparam R
     *      Return type
     *
     * \tparam Args
     *      Argument list
     *
     * \param class_ptr
     *      Pointer to class
     *
     * \param func_ptr
     *      Pointer to non-static member function contained within class C
     *
     * \return
     *      Returns a lambda that when called, calls the non-static member function
     *      'func_ptr' contained within class 'class_ptr'
     */
    PERMUTE_PMF(DEF_GET_METHOD);

    std::function<Signature> function; //!< Function to call
    EVENT_HANDLE handle;                //!< Handle corresponding to the function
};

/*!
 * \brief
 *      Templated event system that holds clients callbacks to be
 *      invoked on elsewhere
 *
 * \tparam FunctionSignature
 *      Function signature of the callbacks to hold
 *      NOTE: All callbacks must be of this type
 *
 * \tparam KeepOrder
 *      Tells the system to invoke callbacks in the same order as they
 *      were hooked. That is, changing the priority when KeepOrder is true is a no-op
 *
 * \tparam Allocator
 *      Allocator for the call list. Must take struct 'Call'
 */
template<typename FunctionSignature, bool KeepOrder = false, typename Allocator = std::allocator<Call<FunctionSignature>>>
class Event
{
    /*!
     * \brief
     *      Assures that all functions in list Fs are non-member functions of a class
     *
     * \tparam Fs
     *      List of function types
     *
     * \return
     *      Returns true if all functions are non-member functions of a class
     *      else static assert
     */
    template<typename ...Fns>
    static constexpr bool class_member_exclusion()
    {
        static_assert(sizeof ...(Fns) > 0, "Calling variadic function with no parameters");
        static_assert(!(... || std::is_member_function_pointer_v<Fns>), "A callback in variadic list Fs... is a class member");
        return true;
    }

    /*!
     * \brief
     *      Used to check if the event can be invoked by the arguments passed in
     *
     * \tparam Args
     *      List of type of arguments attempted to invoke the event with
     *
     * \return
     *      Returns true if the event can be invoked, else static assert
     */
    template<typename ...Args>
    static constexpr bool invocable()
    {
        static_assert(std::is_invocable_v<_Signature , Args...>, "Attempting to invoke event with differing arguments then the event function signature");
        return true;
    }

    /*!
     * \brief
     *      Assures that a type is not contained within a variadic list
     *
     * \tparam Invalid_Type
     *      Invalid type that list Types should not contain
     *
     * \tparam Types
     *      List of types to check
     *
     * \return
     *      Returns true if the type is not within the list, else static assert
     */
    template<typename Invalid_Type, typename ...Types>
    static constexpr bool type_exclusion()
    {
        static_assert(sizeof ...(Types) > 0, "Calling variadic function with no parameters");
        static_assert(!(... || std::is_same_v<Invalid_Type, Types>), "Invalid Type contained in Types variadic list");
        return true;
    }

    /*!
     * \brief
     *      Assures that all functions in variadic Fs list belong to class C
     *
     * \tparam C
     *      Type of class all functions Fs must be apart of
     *
     * \tparam Fs
     *      List of function types
     *
     * \return
     *      Returns true if all functions in list Fs belong to class C
     *      else static assert
     */
    template<typename C, typename ...Fns>
    static constexpr bool class_member_inclusion()
    {
        static_assert(std::is_class_v<C>, "Provided class pointer is not a class");
        static_assert(sizeof ...(Fns) > 0, "Calling variadic function with no parameters");
        static_assert((... && is_member_function_of<C, Fns>::value), "A callback in variadic list Fs... is not a non-static member of class C");
        return true;
    }

    /*!
     * \brief
     *      Assures that all functions in list Fs are callable by this event
     *
     * \tparam Fs
     *      List of functions to assure are compatible with this event function signature
     *
     * \return
     *      Returns true if the list Fs is compatible with this event, else static assert
     */
    template<typename ...Fns>
    static constexpr bool is_same_arg_list()
    {
        static_assert((... && parameter_equivalents<_Signature , Fns>::value), "Attempted to hook a callback that does not have the same parameter list as the event");
        return true;
    }

public:
    using _Signature = FunctionSignature;       //!< Function Signature
    using _Allocator = Allocator;               //!< Event allocator
    using _CallType  = Call<FunctionSignature>; //!< Type of the call wrapper
    static constexpr bool Ordered = KeepOrder;  //!< State of ordering

    /*!
     * \brief
     *      Hooks a non-member function to the event system provided that the type of 'func_ptr'
     *      matches that of 'FUNCTION_SIGNATURE'
     *      NOTE: If you hook the same function with differing priorities, you must specify
     *      the priority, or use the handle when Unhooking
     *
     * \tparam Fn
     *      Type of function
     *
     * \param func_ptr
     *      Pointer to non-member function to be hooked
     *
     * \return
     *      Returns a handle corresponding to the hooked function.
     *      NOTE: Must not be ignored when hooking lambdas, otherwise they become permanently hooked
     */
    template<uint16_t PRIORITY = 0, typename Fn>
    EVENT_HANDLE Hook(Fn *func_ptr)
    VERIFY_TYPE(class_member_exclusion<Fn>() && is_same_arg_list<Fn>())
    {
        uint16_t priority = Ordered ? priority_++ : PRIORITY;
        EVENT_HANDLE handle = MAKE_HANDLE(priority, POINTER_INT_CAST(nullptr), POINTER_INT_CAST(func_ptr));
        callList_[priority].emplace_back(Call<_Signature>(func_ptr, handle));
        return handle;
    }

    /*!
     * \brief
     *      Hooks a lambda function to the event system provided that the type of 'func_ptr'
     *      matches that of 'FUNCTION_SIGNATURE'
     *      NOTE: If you hook the same function with differing priorities, you must specify
     *      the priority, or use the handle when Unhooking
     *
     * \tparam Fn
     *      Type of function
     *
     * \param lambda
     *      The lambda to hook
     *
     * \return
     *      Returns a handle corresponding to the hooked function.
     *      NOTE: Must not be ignored when hooking lambdas, otherwise they become permanently hooked
     */
    template<uint16_t PRIORITY = 0, typename Fn>
    EVENT_HANDLE Hook(Fn &&lambda)
    VERIFY_TYPE(class_member_exclusion<Fn>() && is_same_arg_list<Fn>())
    {
        uint16_t priority = Ordered ? priority_++ : PRIORITY;
        EVENT_HANDLE handle = MAKE_HANDLE(priority, POINTER_INT_CAST(nullptr), POINTER_INT_CAST(&lambda));
        callList_[priority].emplace_back(Call<_Signature>(lambda, handle));
        return handle;
    }

    /*!
     * \brief
     *      Hooks a non-static member function to the event system
     *      NOTE: If you hook the same non-static member function with differing priorities,
     *      you must specify the priority, or use the handle when Unhooking.
     *
     * \tparam C
     *      Type of user defined type
     *
     * \tparam F
     *      Type of function pointer
     *
     * \param class_ref
     *      Reference to the class that has non-static member function 'func_ptr'
     *
     * \param func_ptr
     *      Pointer to non-static member function to hook
     *
     * \return
     *      Returns handle corresponding to non-static member function hooked
     */
    template<uint16_t PRIORITY = 0, typename C, typename Fn>
    EVENT_HANDLE Hook(C *class_ptr, Fn func_ptr)
    VERIFY_TYPE(class_member_inclusion<C, Fn>() && is_same_arg_list<Fn>())
    {
        uint16_t priority = Ordered ? priority_++ : PRIORITY;
        EVENT_HANDLE handle = MAKE_HANDLE(priority, POINTER_INT_CAST(class_ptr), POINTER_INT_CAST(func_ptr));
        callList_[priority].emplace_back(Call<_Signature>(class_ptr, func_ptr, handle));
        return handle;
    }

    /*!
     * \brief
     *      Hooks a cluster of non-member functions to the event system
     *
     * \tparam Fs
     *      List of non-member function types
     *
     * \param func_ptrs
     *      List of pointers to non-member functions to be hooked
     *
     * \return
     *      Returns handle to corresponding to the cluster of non-member functions
     */
    template<uint16_t PRIORITY = 0, typename ...Fns>
    [[nodiscard]] EVENT_HANDLE HookFunctionCluster(Fns&&... func_ptrs)
    VERIFY_TYPE(class_member_exclusion<Fns...>() && type_exclusion<EVENT_HANDLE, Fns...>() && is_same_arg_list<Fns...>())
    {
        PACK_EXPAND(callList_[PRIORITY].emplace_back, Call<_Signature>(func_ptrs, MAKE_HANDLE(PRIORITY, clusterHandle_ + 1, POINTER_INT_CAST(&func_ptrs))))
        return MAKE_HANDLE(PRIORITY, ++clusterHandle_, POINTER_INT_CAST(nullptr));
    }

    /*!
     * \brief
     *      Hooks a cluster of non-static member functions to the event system
     *
     * \tparam C
     *      Type of user defined type
     *
     * \tparam Fs
     *      List of non-static member function types
     *
     * \param class_ref
     *      Reference to the class that has non-static member functions 'func_ptrs'
     *
     * \param func_ptrs
     *      List of pointers to non-static member functions to be hooked
     *
     * \return
     *      Returns handle to corresponding to the cluster of non-static member functions
     */
    template<uint16_t PRIORITY = 0, typename C, typename ...Fns>
    [[nodiscard]] EVENT_HANDLE HookMethodCluster(C *class_ptr, Fns... func_ptrs)
    VERIFY_TYPE(class_member_inclusion<C, Fns...>() && type_exclusion<EVENT_HANDLE, Fns...>() && is_same_arg_list<Fns...>())
    {
        PACK_EXPAND(callList_[PRIORITY].emplace_back, Call<_Signature>(class_ptr, func_ptrs, MAKE_HANDLE(PRIORITY, clusterHandle_ + 1, POINTER_INT_CAST(func_ptrs))))
        return MAKE_HANDLE(PRIORITY, ++clusterHandle_, POINTER_INT_CAST(nullptr));
    }

    /*!
     * \brief
     *      Invokes callbacks hooked to the event
     *      NOTE: Hooking or Unhooking to the same event during the invoke process is undefined
     *      NOTE: Order that the callbacks get invoked is undefined unless specified with a priority
     *
     * \tparam Args
     *      Types of the parameters passed in
     *
     * \param args
     *      Parameters to pass to each of the callback functions
     *      NOTE: Must be the same arguments as the FUNCTION_SIGNATURE
     */
    template<typename ...Args>
    void Invoke(Args... args)
    VERIFY_TYPE(invocable<Args...>())
    {
        for (auto &priority : callList_)
            for (auto &call : priority.second)
                call.function(args...);
    }

    /*!
     * \brief
     *      Unhooks non-member function from event
     *
     * \tparam F
     *      Type of function pointer
     *
     * \param func_ptr
     *      Pointer to non-member function to unhook
     *      Note: 'func_ptr' must not be a lambda
     */
    template<uint16_t PRIORITY = 0, typename Fn>
    void Unhook(Fn func_ptr)
    VERIFY_TYPE(class_member_exclusion<Fn>())
    {
        RemoveCall(MAKE_HANDLE(PRIORITY, POINTER_INT_CAST(nullptr), POINTER_INT_CAST(func_ptr)));
    }

    /*!
     * \brief
     *      Unhooks non-static member function from event
     *
     * \tparam C
     *      Type of user defined type
     *
     * \tparam F
     *      Type of non-static member function
     *
     * \param class_ptr
     *      Address of the user defined type that contains the non-static member function 'func_ptr'
     *
     * \param func_ptr
     *      Pointer to non-static member function to unhook
     */
    template<uint16_t PRIORITY = 0, typename C, typename Fn>
    void Unhook(C *class_ptr, Fn func_ptr)
    VERIFY_TYPE(class_member_inclusion<C, Fn>())
    {
        RemoveCall(MAKE_HANDLE(PRIORITY, POINTER_INT_CAST(class_ptr), POINTER_INT_CAST(func_ptr)));
    }

    /*!
     * \brief
     *      Unhooks a function that a handle corresponds to
     *      NOTE: Attempting to use a handle given for a cluster
     *            will not remove anything
     *
     * \param handle
     *      Handle corresponding to the function to unhook
     */
    void Unhook(EVENT_HANDLE handle)
    {
        RemoveCall(handle);
    }

    /*!
     * \brief
     *      Unhooks the cluster of functions corresponding to the handle
     *
     * \param handle
     *      Handle corresponding to the cluster of cuntions hooked by Hook##Cluster functions
     */
    void UnhookCluster(EVENT_HANDLE handle)
    {
        RemoveCluster(handle);
    }

    /*!
     * \brief
     *      Unhooks all non-static member functions hooked with the
     *      user defined type
     *
     * \tparam C
     *      Type of user defined type
     *
     * \param class_ptr
     *      Address of the class has non-static member functions hooked to event
     */
    template<uint16_t PRIORITY = 0, typename C>
    void UnhookClass(C *class_ptr)
    {
        static_assert(std::is_class_v<C>, "Class pointer provided not a pointer to a class");
        RemoveCluster(MAKE_HANDLE(PRIORITY, POINTER_INT_CAST(class_ptr), POINTER_INT_CAST(nullptr)));
    }

    /*!
     * \brief
     *      Unhooks a list of non-member functions hooked to the event
     *
     * \tparam Fs
     *      List of non-member function types
     *
     * \param func_ptrs
     *      List of non-member functions to unhook from event
     */
    template<typename ...Fns>
    void UnhookFunctions(Fns ...func_ptrs)
    VERIFY_TYPE(class_member_exclusion<Fns...>() && type_exclusion<EVENT_HANDLE, Fns...>())
    {
        PACK_EXPAND(Unhook, func_ptrs)
    }

    /*!
     * \brief
     *      Unhooks a list of non-static member functions both hooked to event
     *      and contained within the class of type C
     *
     * \tparam C
     *      Type of user defined type
     *
     * \tparam Fs
     *      List of non-static member function types
     *
     * \param class_ptr
     *      Address of the class that contains the non-static member functions
     *      to unhook
     *
     * \param func_ptrs
     *      List of non-static member functions contained in class C to unhook from event
     *
     */
    template<typename C, typename ...Fns>
    void UnhookMethods(C *class_ptr, Fns ...func_ptrs)
    VERIFY_TYPE(class_member_inclusion<C, Fns...>() && type_exclusion<EVENT_HANDLE, Fns...>())
    {
        PACK_EXPAND(Unhook, class_ptr, func_ptrs)
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
        for (auto &call_list : callList_)
            size += call_list.second.size();
        return size;
    }

    /*!
     * \brief
     *      Clears the call list
     */
    void Clear()
    {
        for (auto &call_list : callList_)
            call_list.second.clear();
        clusterHandle_ = 0;
    }
private:
    struct USet; struct CallHash; // forward declare

    //! Type of priority to callback list
    using CallListType = std::map<unsigned int, USet>;

    CallListType callList_;      //!< List of callbacks
    uint16_t priority_ = 0;      //!< Increases on each Hook* when KeepOrder is true
    uint16_t clusterHandle_ = 0; //!< Cluster handle to differ from class address

    /*!
     * \brief
     *      Unhooks a cluster handle from the call list
     *
     * \param cluster
     *      Handle to cluster of events to unhook
     */
    void RemoveCluster(EVENT_HANDLE handle)
    {
        EVENT_HANDLE cluster = GET_CLUSTER(handle);
        auto &call_list = callList_[GET_PRIORITY(handle)];
        for (auto it = call_list.begin(); it != call_list.end();)
            if (cluster == GET_CLUSTER(it->handle))
                it = call_list.erase(it);
            else
                ++it;
    }

    /*!
     * \brief
     *      Unhooks a handle from the call list
     *
     * \param handle
     *      Handle to the function to unhook
     */
    void RemoveCall(EVENT_HANDLE handle)
    {
        auto &call_list = callList_[GET_PRIORITY(handle)];
        auto call = std::find(call_list.begin(), call_list.end(), handle);
        if (call != call_list.end()) call_list.erase(call);
    }

    /*!
     * \brief
     *      Converts a pointer into an std::uintptr_t for the EVENT_HANDLE
     *
     * \tparam T
     *      Type of pointer
     *
     * \param t
     *      Pointer to hash
     *
     * \return
     *      Returns the address of the pointer as an int
     */
    template<typename T>
    static constexpr std::uintptr_t POINTER_INT_CAST(T t)
    {
        return reinterpret_cast<std::uintptr_t>(*reinterpret_cast<void**>(&t));
    }

    /*!
     * \brief
     *      Hash functor used in unordered_set
     */
    struct CallHash
    {
        /*!
         * \brief
         *      Function call operator
         *
         * \param call
         *      Call to hash
         *
         * \return
         *      Returns the handle of the call
         */
        EVENT_HANDLE operator()(const Call<_Signature>& call) const
        {
            return call.handle;
        }
    };

    /*!
     * \brief
     *      Wrapper around an unordered_set to standard the emplace_back function
     */
    struct USet : public std::unordered_set<Call<_Signature>, CallHash, std::equal_to<Call<_Signature>>, _Allocator>
    {
        /*!
         * \brief
         *      Wrapper around emplace to work with std::vector
         *
         * \tparam Args
         *      Argument types to the constructor of Call
         *
         * \param args
         *      Arguments to the constructor of Call data type
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
     *      Overload for non-static non-ellipsis member functions
     */
#define DEF_IS_MEMBER_FUNCTION_OF(CV_REF_NOEXCEPT_OPT) \
      template<typename C, typename R, typename... Args> \
      struct is_member_function_of<C, R(C::*)(Args...) CV_REF_NOEXCEPT_OPT> \
      { \
          static constexpr bool value = true; \
      }

    /*!
     * \brief
     *     Overload for non-static ellipsis member functions
     */
#define DEF_IS_MEMBER_FUNCTION_OF_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
      template<typename C, typename R, typename... Args> \
      struct is_member_function_of<C, R(C::*)(Args..., ...) CV_REF_NOEXCEPT_OPT> \
      { \
          static constexpr bool value = true; \
      }

    /*!
     * \brief
     *      Generates all overloads for member functions and their decorators
     *
     * \tparam C
     *      Class type
     *
     * \tparam R
     *      Return type
     *
     * \tparam Args
     *      Argument type list
     */
    PERMUTE_PMF(DEF_IS_MEMBER_FUNCTION_OF);

    /*!
     * \brief
     *      Base case, is a lambda, check if it can be constructed as an std::function
     */
    template<typename COMP, typename Fn>
    struct parameter_equivalents
    { static constexpr bool value = std::is_constructible_v<std::function<_Signature>, Fn>; };

    /*!
     * \brief
     *     Non-member function overload to see if COMP can be invoked with arguments Args
     *
     * \tparam COMP
     *     Function type to be invoked
     *
     * \tparam R
     *     Return type of non-member function
     *
     * \tparam Args
     *     Argument list of non-member function
     */
    template<typename COMP, typename R, typename ...Args>
    struct parameter_equivalents<COMP, R(*)(Args...)>
    { static constexpr bool value = std::is_invocable_v<COMP, Args...>; };

    /*!
     * \brief
     *      Overload for non-static non-ellipsis member function
     */
#define DEF_PARAMETER_EQUIVALENTS(CV_REF_NOEXCEPT_OPT) \
    template<typename COMP, typename C, typename R, typename ...Args> \
    struct parameter_equivalents<COMP, R(C::*)(Args...) CV_REF_NOEXCEPT_OPT> \
    { static constexpr bool value = std::is_invocable_v<COMP, Args...>; };

    /*!
     * \brief
     *      Overload for non-static ellipsis member function
     */
#define DEF_PARAMETER_EQUIVALENTS_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
    template<typename COMP, typename C, typename R, typename ...Args> \
    struct parameter_equivalents<COMP, R(C::*)(Args..., ...) CV_REF_NOEXCEPT_OPT> \
    { static constexpr bool value = std::is_invocable_v<COMP, Args...>; };

    /*!
     * \brief
     *      Generates all overloads to check if COMP can be invoked with
     *      the arguments of Args
     *
     * \tparam COMP
     *      Function signature to be invoked with arguments Args
     *
     * \tparam C
     *      Type of class
     *
     * \tparam R
     *      Return type
     *
     * \tparam Args
     *      List of argument types
     */
    PERMUTE_PMF(DEF_PARAMETER_EQUIVALENTS);
};

#endif

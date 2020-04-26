//                                                                 .---.    //
//                                                                /  .  \   //
//                                                               |\_/|   |  //
//                                                               |   |  /|  //
//    .----------------------------------------------------------------' |  //
//   /  .-.  /*!\author         Tristan Bouchard                       */|  //
//  |  /   \ /*!\file              Event.hpp                           */|  //
//  | |\_.  |/*!\date              4/20/2020                           */|  //
//  |\|  | /|/*!                                                       */|  //
//  | `---' |/*!\brief This file contains the implementation for an    */|  //
//  |       |/*!            templated C++ Event system                 */|  //
//  |       |/*!                                                       */|  //
//  |       |/*!\par link: https://github.com/BeOurQuest/Events.git    */|  //
//  |       |/*!                                                        /   //
//  |       |----------------------------------------------------------'    //
//  \       |                                                               //
//   \     /                                                                //
//    `---'                                                                 //
#ifndef EVENTS_HPP
#define EVENTS_HPP
#include <unordered_set> // unordered_set
#include <functional>    // function
#include <assert.h>      // assert
#include <cstdint>       // uint64_t
#include <vector>        // vector
#include <mutex>         // mutex
#include <map>           // map

//! For variadic template expansion
#define PACK_EXPAND(function, ...) ((void)function(__VA_ARGS__), ...);

/*!
 * \brief
 *      Handle to a function within the event call list
 *      Layout of EVENT_HANDLE memory:
 *                                              |    32bits    |       32bits     |
 *      if non-member function                = | 0            | &function        |
 *      if non-static member function         = | &class       | &member_function |
 *      if non-member function cluster        = | unique val   | &function        |
 *      if non-static member function cluster = | unique val   | &member_function |
 */
using EVENT_HANDLE = uint64_t;

// ! Mask for getting the cluster
#define EVENT_CLUSTER_MASK 0b11111111111111111111111111111111'00000000000000000000000000000000

//! Mask for getting the id
#define EVENT_ID_MASK 0b00000000000000000000000000000000'11111111111111111111111111111111

//! Gets the cluster from a handle
#define GET_CLUSTER(handle) (EVENT_HANDLE(handle) & EVENT_CLUSTER_MASK)

//! Get the id from the handle
#define GET_ID(handle) (EVENT_HANDLE(handle) & EVENT_ID_MASK)

//! Constructs a handle with the last 4 bytes has the cluster and the first 4 be the ID
#define GET_HANDLE(cluster, id) ((EVENT_HANDLE(cluster) << sizeof(uint32_t) * 8) | GET_ID(id))

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
 *      Templated event system that holds clients callbacks to be
 *      invoked on elsewhere
 *
 * \tparam FUNCTION_SIGNATURE
 *      Function signature of the callbacks to hold
 *      NOTE: All callbacks must be of this type
 *
 * \tparam KEEP_ORDER
 *      Tells the system to invoke callbacks in the same order as they
 *      were hooked.
 */
template<typename FUNCTION_SIGNATURE, bool KEEP_ORDER = false>
class Event
{
  struct Call; // forward declare callback data type
  public:
    /*!
     * \brief
     *      Hooks a non-member function to the event system provided that the type of 'func_ptr'
     *      matches that of 'FUNCTION_SIGNATURE'
     *
     * \tparam F
     *      Type of function
     *
     * \param func_ptr
     *      Pointer to non-member function to be hooked
     *
     * \return
     *      Returns a handle corresponding to the hooked function.
     *      NOTE: Must not be ignored when hooking lambdas, otherwise they become permanently hooked
     */
    template<typename F>
    EVENT_HANDLE Hook(F &&func_ptr)
    VERIFY_TYPE(class_member_exclusion<F>() && is_same_arg_list<F>())
    {
      EVENT_HANDLE handle = GET_HANDLE(POINTER_INT_CAST(nullptr), POINTER_INT_CAST(&func_ptr));
      callList_.emplace_back(Call(func_ptr, handle));
      return handle;
    }

    /*!
     * \brief
     *      Hooks a non-static member function to the event system
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
    template<typename C, typename F>
    EVENT_HANDLE Hook(C &class_ref, F func_ptr)
    VERIFY_TYPE(class_member_inclusion<C, F>() && is_same_arg_list<F>())
    {
      EVENT_HANDLE handle = GET_HANDLE(POINTER_INT_CAST(&class_ref), POINTER_INT_CAST(func_ptr));
      callList_.emplace_back(Call(&class_ref, func_ptr, handle));
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
    template<typename ...Fs>
    [[nodiscard]] EVENT_HANDLE HookFunctionCluster(Fs&&... func_ptrs)
    VERIFY_TYPE(class_member_exclusion<Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>() && is_same_arg_list<Fs...>())
    {
      PACK_EXPAND(callList_.emplace_back, Call(func_ptrs, GET_HANDLE(clusterHandle_ + 1, POINTER_INT_CAST(&func_ptrs))))
      return GET_HANDLE(++clusterHandle_, POINTER_INT_CAST(nullptr));
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
    template<typename C, typename ...Fs>
    [[nodiscard]] EVENT_HANDLE HookMethodCluster(C &class_ref, Fs... func_ptrs)
    VERIFY_TYPE(class_member_inclusion<C, Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>() && is_same_arg_list<Fs...>())
    {
      PACK_EXPAND(callList_.emplace_back, Call(&class_ref, func_ptrs, GET_HANDLE(clusterHandle_ + 1, func_ptrs)))
      return GET_HANDLE(++clusterHandle_, POINTER_INT_CAST(nullptr));
    }

    /*!
     * \brief
     *      Invokes callbacks hooked to the event
     *      NOTE: Hooking or Unhooking to the same event during the invoke process is undefined
     *      NOTE: Thread safe
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
      std::lock_guard<std::mutex> lk(m_mutex[this]);
      for (auto &call : callList_)
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
     */
    template<typename F>
    void Unhook(F func_ptr)
    VERIFY_TYPE(class_member_exclusion<F>())
    {
      RemoveCall(GET_HANDLE(POINTER_INT_CAST(nullptr), POINTER_INT_CAST(func_ptr)));
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
     * \param class_ref
     *      Reference to user defined type that contains the non-static member function 'func_ptr'
     *
     * \param func_ptr
     *      Pointer to non-static member function to unhook
     */
    template<typename C, typename F>
    void Unhook(C &class_ref, F func_ptr)
    VERIFY_TYPE(class_member_inclusion<C, F>())
    {
      RemoveCall(GET_HANDLE(POINTER_INT_CAST(&class_ref), POINTER_INT_CAST(func_ptr)));
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
      RemoveCluster(GET_CLUSTER(handle));
    }

    /*!
     * \brief
     *      Unhooks all non-static member functions hooked with the
     *      user defined type
     *
     * \tparam C
     *      Type of user defined type
     *
     * \param class_ref
     *      Reference to class has non-static member functions hooked to event
     */
    template<typename C>
    void UnhookClass(C &class_ref)
    {
      static_assert(std::is_class_v<C>, "Class pointer provided not a pointer to a class");
      RemoveCluster(GET_HANDLE(POINTER_INT_CAST(&class_ref), POINTER_INT_CAST(nullptr)));
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
    template<typename ...Fs>
    void Unhook_fs(Fs ...func_ptrs)
    VERIFY_TYPE(class_member_exclusion<Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
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
     * \param class_ref
     *      Reference to class that contains the non-static member functions
     *      to unhook
     *
     * \param func_ptrs
     *      List of non-static member functions contained in class C to unhook from event
     *
     */
    template<typename C, typename ...Fs>
    void Unhook_m(C &class_ref, Fs ...func_ptrs)
    VERIFY_TYPE(class_member_inclusion<C, Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(Unhook, &class_ref, func_ptrs)
    }

    /*!
     * \brief
     *      Getter for how many callbacks are stored within this event
     *
     * \return
     *      Returns number of callbacks hooked to this event
     */
    size_t CallListSize() const
    {
        return callList_.size();
    }

    /*!
     * \brief
     *      Removes the this pointer from static map contained within invoke
     */
    ~Event()
    {
      auto it = m_mutex.find(this);
      if (it != m_mutex.end())
        m_mutex.erase(it);
    }
  private:
    struct USet; struct CallHash; // forward declare

    //! Type of callback list
    typedef typename std::conditional<KEEP_ORDER, std::vector<Call>, USet>::type CallListType;

    //! Static map of mutex to handle thread safety for invoking
    static inline std::map<Event *, std::mutex> m_mutex;

    CallListType callList_;          //!< List of callbacks
    EVENT_HANDLE clusterHandle_ = 0; //!< Cluster handle to differ from class address

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
      static_assert(std::is_invocable_v<FUNCTION_SIGNATURE, Args...>,
                    "Attempting to invoke event with differing arguments then the event function signature");
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
      static_assert(sizeof ...(Types) > 0,
                    "Calling variadic function with no parameters");
      static_assert(!(... || std::is_same_v<Invalid_Type, Types>),
                    "Invalid Type contained in Types variadic list");
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
    template<typename C, typename ...Fs>
    static constexpr bool class_member_inclusion() // assures Fs... are members of class C
    {
      static_assert(std::is_class_v<C>,
                    "Provided class pointer is not a class");
      static_assert(sizeof ...(Fs) > 0,
                    "Calling variadic function with no parameters");
      static_assert((... && is_member_function_of_v<C, Fs>),
                    "A callback in variadic list Fs... is not a non-static member of class C");
      return true;
    }

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
    template<typename ...Fs>
    static constexpr bool class_member_exclusion() // assures Fs... are not members of a class
    {
      static_assert(sizeof ...(Fs) > 0,
                    "Calling variadic function with no parameters");
      static_assert(!(... || std::is_member_function_pointer_v<Fs>),
                    "A callback in variadic list Fs... is a class member");
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
    template<typename ...Fs>
    static constexpr bool is_same_arg_list()
    {
      static_assert((... && parameter_equivalents_v<FUNCTION_SIGNATURE, Fs>),
                    "Attempted to hook a callback that does not have the same parameter list as the event");
      return true;
    }

    /*!
     * \brief
     *      Unhooks a cluster handle from the call list
     *
     * \param cluster
     *      Handle to cluster of events to unhook
     */
    void RemoveCluster(EVENT_HANDLE cluster)
    {
      for (auto it = callList_.begin(); it != callList_.end();)
        if (cluster == GET_CLUSTER(it->handle))
          it = callList_.erase(it);
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
      auto call = std::find(callList_.begin(), callList_.end(), handle);
      if (call != callList_.end()) callList_.erase(call);
    }

    /*!
     * \brief
     *      Callback wrapper
     */
    struct Call
    {
      /*!
       * \brief
       *      Default Constructor
       */
      Call()
      : function([](){}), handle(EVENT_HANDLE(0))
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
      template<typename F>
      Call(F func_ptr, EVENT_HANDLE handle)
      : function(func_ptr), handle(handle)
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
      template<typename C, typename F>
      Call(C class_ptr, F func_ptr, EVENT_HANDLE handle)
      : function(GetMethod(class_ptr, func_ptr)), handle(handle)
      {}

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
      bool operator==(const Call& other) const
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
      bool operator==(EVENT_HANDLE handle) const
      {
        return this->handle == handle;
      }

      /*!
       * \brief
       *      Implicit conversion operator
       *
       * \return
       *      Returns handle of call
       */
      operator EVENT_HANDLE()
      {
        return handle;
      }

      std::function<FUNCTION_SIGNATURE> function; //!< Function to call
      EVENT_HANDLE handle;                        //!< Handle corresponding to the function
    };

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
      EVENT_HANDLE operator()(const Call& call) const
      {
        return call.handle;
      }
    };

    /*!
     * \brief
     *      Wrapper around an unordered_set to standard the emplate_back function
     */
    struct USet : public std::unordered_set<Call, CallHash>
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
        assert(this->emplace(args...).second && "ERROR : Duplicate function hooked to event");
      }
    };

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
     *      Checks if a function F passed in is apart of a class C
     *      is true if F is a non-static member of class C
     *
     * \tparam C
     *      Class type
     *
     * \tparam F
     *      Function type
     */
    template<typename C, typename F>
    static inline constexpr bool is_member_function_of_v = is_member_function_of<C, F>::value;

    /*!
     * \brief
     *      Base case, is a lambda, check if it can be constructed as an std::function
     */
    template<typename COMP, typename F>
    struct parameter_equivalents
    { static constexpr bool value = std::is_constructible_v<std::function<FUNCTION_SIGNATURE>, F>; };

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

    /*!
     * \brief
     *      Checks to see if the parameters of F are that of the parameters of COMP
     *      is true if they have the same argument list
     *
     * \tparam COMP
     *     Function signature to be invoked with argument list from F
     *
     * \tparam F
     *     Function type to compare against COMP
     */
    template<typename COMP, typename F>
    static inline constexpr bool parameter_equivalents_v = parameter_equivalents<COMP, F>::value;
};

#endif

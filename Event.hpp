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

//! Handle to a function within the event call list
using EVENT_HANDLE = uint64_t;

//! Constructs a handle with the last 4 bytes has the cluster and the first 4 be the ID
#define GET_HANDLE(cluster, id) ((EVENT_HANDLE(cluster) << sizeof(uint32_t)) | EVENT_HANDLE(uint32_t(id)))

//! Get the cluster
#define GET_CLUSTER(handle) (EVENT_HANDLE(handle) >> sizeof(uint32_t))

//! Get the id
#define GET_ID(handle) ((EVENT_HANDLE(handle) << sizeof(uint32_t)) >> sizeof(uint32_t))

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

// Unhooks should allow both event handles as well as the pointers itself, but not both at the same time
// Fix issue with calling class pointer as normal function
// TODO: make network safe (might have to be a rewrite)
// TODO: make thread safe
// TODO: make cross platform (clang, gcc, msvc)
// TODO: Hooking or Unhooking to the same event during the invoke process is undefined

// unhook class removes all the classes methods
// unhook just looks at the first 4 bytes

// function clusters use the last 4 bytes

// NOTE: Cluster ids must be different than the class pointer

// if function |0|&function|
// if member function |&class|&member
// if function cluster |?|&function
// if member cluster |?|&member

/*!
 * \brief
 * \tparam FUNCTION_SIGNATURE
 * \tparam KEEP_ORDER
 */
template<typename FUNCTION_SIGNATURE, bool KEEP_ORDER = false>
class Event
{
  /*!
   * \brief
   */
  struct Call;

  public:
    /*!
     * \brief
     * \tparam F
     * \param func_ptr
     * \return
     */
    template<typename F>
    EVENT_HANDLE Hook(F func_ptr)
    VERIFY_TYPE(class_member_exclusion<F>())
    {
      EVENT_HANDLE handle = GET_HANDLE(POINTER_INT_CAST(nullptr), POINTER_INT_CAST(func_ptr));
      callList_.emplace_back(Call(func_ptr, handle));
      return handle;
    }

    /*!
     * \brief
     * \tparam C
     * \tparam F
     * \param class_ptr
     * \param func_ptr
     * \return
     */
    template<typename C, typename F>
    EVENT_HANDLE Hook(C &class_ptr, F func_ptr)
    VERIFY_TYPE(class_member_inclusion<C, F>())
    {
      EVENT_HANDLE handle = GET_HANDLE(POINTER_INT_CAST(&class_ptr), POINTER_INT_CAST(func_ptr));
      callList_.emplace_back(Call(&class_ptr, func_ptr, handle));
      return handle;
    }

    /*!
     * \brief
     * \tparam Fs
     * \param func_ptrs
     * \return
     */
    template<typename ...Fs>
    [[nodiscard]] EVENT_HANDLE HookFunctionCluster(Fs... func_ptrs)
    VERIFY_TYPE(class_member_exclusion<Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(callList_.emplace_back, Call(func_ptrs, GET_HANDLE(clusterHandle_ + 1, POINTER_INT_CAST(func_ptrs))))
      return GET_HANDLE(++clusterHandle_, POINTER_INT_CAST(nullptr));
    }

    /*!
     * \brief
     * \tparam C
     * \tparam Fs
     * \param class_ptr
     * \param func_ptrs
     * \return
     */
    template<typename C, typename ...Fs>
    [[nodiscard]] EVENT_HANDLE HookMethodCluster(C &class_ptr, Fs... func_ptrs)
    VERIFY_TYPE(class_member_inclusion<C, Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(callList_.emplace_back, Call(&class_ptr, func_ptrs, GET_HANDLE(clusterHandle_ + 1, func_ptrs)))
      GET_HANDLE(++clusterHandle_, POINTER_INT_CAST(nullptr));
    }

    /*!
     * \brief
     * \tparam Args
     * \param args
     */
    template<typename ...Args>
    void Invoke(Args... args)
    VERIFY_TYPE(invocable<FUNCTION_SIGNATURE, Args...>())
    {
      static std::map<Event *, std::mutex> m_mutex;
      std::lock_guard<std::mutex> lk(m_mutex[this]);
      for (auto &call : callList_)
        call.function(args...);
    }

    /*!
     * \brief
     * \tparam F
     * \param func_ptr
     */
    template<typename F>
    void Unhook(F func_ptr)
    VERIFY_TYPE(class_member_exclusion<F>())
    {
      RemoveCall(GET_HANDLE(POINTER_INT_CAST(nullptr), POINTER_INT_CAST(func_ptr)));
    }

    /*!
     * \brief
     * \tparam C
     * \tparam F
     * \param class_ptr
     * \param func_ptr
     */
    template<typename C, typename F>
    void Unhook(C &class_ptr, F func_ptr)
    VERIFY_TYPE(class_member_inclusion<C, F>())
    {
      RemoveCall(GET_HANDLE(POINTER_INT_CAST(&class_ptr), POINTER_INT_CAST(func_ptr)));
    }

    /*!
     * \brief
     * \param handle
     */
    void Unhook(EVENT_HANDLE handle)
    {
      RemoveCall(handle);
    }

    /*!
     * \brief
     * \param handle
     */
    void UnhookCluster(EVENT_HANDLE handle)
    {
      RemoveCluster(GET_CLUSTER(handle));
    }

    /*!
     * \brief
     * \tparam C
     * \param class_ptr
     */
    template<typename C>
    void UnhookClass(C &class_ptr)
    {
      static_assert(std::is_class_v<C>, "Class pointer provided not a pointer to a class");
      RemoveCluster(GET_HANDLE(POINTER_INT_CAST(&class_ptr), POINTER_INT_CAST(nullptr)));
    }

    /*!
     * \brief
     * \tparam Fs
     * \param func_ptrs
     */
    template<typename ...Fs>
    void Unhook_fv(Fs ...func_ptrs)
    VERIFY_TYPE(class_member_exclusion<Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(Unhook, func_ptrs)
    }

    /*!
     * \brief
     * \tparam C
     * \tparam Fs
     * \param class_ptr
     * \param func_ptrs
     */
    template<typename C, typename ...Fs>
    void Unhook_mv(C &class_ptr, Fs ...func_ptrs)
    VERIFY_TYPE(class_member_inclusion<C, Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(Unhook, &class_ptr, func_ptrs)
    }

    /*!
     * \brief
     * \return
     */
    size_t CallListSize() const
    {
        return callList_.size();
    }

  private:
    struct USet; struct CallHash; //!
    typedef typename std::conditional<KEEP_ORDER, std::vector<Call>, USet>::type CallListType; //!
    CallListType callList_; //!
    EVENT_HANDLE clusterHandle_ = 0; //!

    /*!
     * \brief
     * \tparam F
     * \tparam Args
     * \return
     */
    template<typename F, typename ...Args>
    static constexpr bool invocable()
    {
      static_assert(std::is_invocable_v<F, Args...>, "Function arguments provided does not match function args of Event class");
      return true;
    }

    /*!
     * \brief
     * \tparam Invalid_Type
     * \tparam Types
     * \return
     */
    template<typename Invalid_Type, typename ...Types>
    static constexpr bool type_exclusion() // assures a type is not contained within Types...
    {
      static_assert(sizeof ...(Types) > 0, "Calling variadic function with no parameters");
      static_assert(!std::disjunction_v<std::is_same<Invalid_Type, Types>...>, "Invalid Type contained in Types variadic list");
      return true;
    }

    /*!
     * \brief
     * \tparam C
     * \tparam Fs
     * \return
     */
    template<typename C, typename ...Fs>
    static constexpr bool class_member_inclusion() // assures Fs... are members of class C
    {
      static_assert(std::is_class_v<C>, "Provided class pointer is not a class");
      static_assert(sizeof ...(Fs) > 0, "Calling variadic function with no parameters");
      static_assert((... && is_member_function_of_v<C, Fs>), "A function in variadic list Fs... is not a non-static member of class C");
      return true;
    }

    /*!
     * \brief
     * \tparam Fs
     * \return
     */
    template<typename ...Fs>
    static constexpr bool class_member_exclusion() // assures Fs... are not members of a class
    {
      static_assert(sizeof ...(Fs) > 0, "Calling variadic function with no parameters");
      static_assert(!std::disjunction_v<std::is_member_function_pointer<Fs>...>, "A function in variadic list Fs... is a class member");
      return true;
    }

    /*!
     * \brief
     * \param cluster
     */
    void RemoveCluster(EVENT_HANDLE cluster)
    {
      for (auto it = callList_.begin(); it != callList_.end();)
        if (cluster == GET_CLUSTER(it.handle))
          it = callList_.erase(it);
        else
          ++it;
    }

    /*!
     * \brief
     * \param handle
     */
    void RemoveCall(EVENT_HANDLE handle)
    {
      auto call = std::find(callList_.begin(), callList_.end(), handle);
      if (call != callList_.end()) callList_.erase(call);
    }

    /*!
     * \brief
     */
    struct Call
    {
      /*!
       * \brief
       */
      Call()
      : function([](){}), handle(EVENT_HANDLE(0))
      {}

      /*!
       * \brief
       * \tparam F
       * \param func_ptr
       * \param handle
       */
      template<typename F>
      Call(F func_ptr, EVENT_HANDLE handle)
      : function(func_ptr), handle(handle)
      {}

      /*!
       * \brief
       * \tparam C
       * \tparam F
       * \param class_ptr
       * \param func_ptr
       * \param handle
       */
      template<typename C, typename F>
      Call(C class_ptr, F func_ptr, EVENT_HANDLE handle)
      : function(GetMethod(class_ptr, func_ptr)), handle(handle)
      {}

      /*!
       * \brief
       */
      #define DEF_GET_METHOD(CV_REF_NOEXCEPT_OPT) \
        template<typename C, typename R, typename ...Args> \
        auto GetMethod(C *class_ptr, R (C::*func_ptr)(Args...) CV_REF_NOEXCEPT_OPT) \
        { \
          return [class_ptr, func_ptr] (Args... args) mutable { (void)(std::mem_fn(func_ptr)(class_ptr, args...)); }; \
        }

      /*!
       * \brief
       */
      #define DEF_GET_METHOD_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
        template<typename C, typename R, typename ...Args> \
        auto GetMethod(C *class_ptr, R (C::*func_ptr)(Args..., ...) CV_REF_NOEXCEPT_OPT) \
        { \
          return [class_ptr, func_ptr](Args... args) mutable { (void)(std::mem_fn(func_ptr)(class_ptr, args...)); }; \
        }

      /*!
       * \brief
       * \tparam C
       * \tparam R
       * \tparam Args
       * \param class_ptr
       * \param func_ptr
       * \return
       */
      PERMUTE_PMF(DEF_GET_METHOD);

      /*!
       * \brief
       * \param other
       * \return
       */
      bool operator==(const Call& other) const
      {
        return handle == other.handle;
      }

      /*!
       * \brief
       * \return
       */
      operator EVENT_HANDLE()
      {
        return handle;
      }

      std::function<FUNCTION_SIGNATURE> function; //!
      EVENT_HANDLE handle; //!
    };

    /*!
     * \brief
     */
    struct CallHash
    {
      /*!
       * \brief
       * \param call
       * \return
       */
      EVENT_HANDLE operator()(const Call& call) const
      {
        return call.handle;
      }
    };

    /*!
     * \brief
     */
    struct USet : public std::unordered_set<Call, CallHash>
    {
      /*!
       * \brief
       *
       */
      USet() = default;

      /*!
       * \brief
       * \tparam Args
       * \param args
       */
      template<typename ...Args>
      void emplace_back(Args... args)
      {
        assert(this->emplace(args...).second && "ERROR : Duplicate function hooked to event");
      }
    };

    /*!
     * \brief
     * \tparam T
     * \param t
     * \return
     */
    template<typename T>
    static constexpr std::uintptr_t POINTER_INT_CAST(T t)
    {
      return reinterpret_cast<std::uintptr_t>(*reinterpret_cast<void**>(&t));
    }

    /*!
     * \brief
     */
    template<typename, typename>
    struct is_member_function_of
    {
      static constexpr bool value = false;
    };

    /*!
     * \brief
     */
    #define DEF_IS_MEMBER_FUNCTION_OF(CV_REF_NOEXCEPT_OPT) \
      template<typename T, typename Ret, typename... Args> \
      struct is_member_function_of<T, Ret (T::*)(Args...) CV_REF_NOEXCEPT_OPT> \
      { \
          static constexpr bool value = true; \
      }

    /*!
     * \brief
     */
    #define DEF_IS_MEMBER_FUNCTION_OF_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
      template<typename T, typename Ret, typename... Args> \
      struct is_member_function_of<T, Ret (T::*)(Args..., ...) CV_REF_NOEXCEPT_OPT> \
      { \
          static constexpr bool value = true; \
      }

    /*!
     * \brief
     * \tparam T
     * \tparam Ret
     * \tparam Args
     */
    PERMUTE_PMF(DEF_IS_MEMBER_FUNCTION_OF);

    /*!
     * \brief
     * \tparam T
     * \tparam Fn
     */
    template<typename T, typename Fn>
    static inline constexpr bool is_member_function_of_v = is_member_function_of<T, Fn>::value;
};

#endif

//                                                                 .---.    //
//                                                                /  .  \   //
//                                                               |\_/|   |  //
//                                                               |   |  /|  //
//    .----------------------------------------------------------------' |  //
//   /  .-.  /*!\author            Tristan Bouchard                    */|  //
//  |  /   \ /*!\file              event_class.h                       */|  //
//  | |\_.  |/*!\date              8/19/2019                           */|  //
//  |\|  | /|/*!                                                       */|  //
//  | `---' |/*!\brief This file contains the implementation for the   */|  //
//  |       |/*!       event class used in events                      */|  //
//  |       |/*!                                                       */|  //
//  |       |/*!                                                        /   //
//  |       |----------------------------------------------------------'    //
//  \       |                                                               //
//   \     /                                                                //
//    `---'                                                                 //
#ifndef EVENTS_HPP
#define EVENTS_HPP
#include <unordered_set>
#include <functional>
#include <assert.h>
#include <cstdint>
#include <vector>
#include <mutex>
#include <map>

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

//! For type checking
#define VERIFY_TYPE noexcept

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

template<typename FUNCTION_SIGNATURE, bool KEEP_ORDER = false>
class Event
{
  struct Call;
  public:
    template<typename F>
    EVENT_HANDLE Hook(F func_ptr)
    VERIFY_TYPE(class_member_exclusion<F>())
    {
      EVENT_HANDLE handle = GET_HANDLE(POINTER_INT_CAST(nullptr), POINTER_INT_CAST(func_ptr));
      callList_.emplace_back(Call(func_ptr, handle));
      return handle;
    }

    template<typename C, typename F>
    EVENT_HANDLE Hook(C &class_ptr, F func_ptr)
    VERIFY_TYPE(class_member_inclusion<C, F>())
    {
      EVENT_HANDLE handle = GET_HANDLE(POINTER_INT_CAST(&class_ptr), POINTER_INT_CAST(func_ptr));
      callList_.emplace_back(Call(&class_ptr, func_ptr, handle));
      return handle;
    }

    template<typename ...Fs>
    [[nodiscard]] EVENT_HANDLE HookFunctionCluster(Fs... func_ptrs)
    VERIFY_TYPE(class_member_exclusion<Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(callList_.emplace_back, Call(func_ptrs, GET_HANDLE(clusterHandle_ + 1, POINTER_INT_CAST(func_ptrs))))
      return GET_HANDLE(++clusterHandle_, POINTER_INT_CAST(nullptr));
    }

    template<typename C, typename ...Fs>
    [[nodiscard]] EVENT_HANDLE HookMethodCluster(C &class_ptr, Fs... func_ptrs)
    VERIFY_TYPE(class_member_inclusion<C, Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(callList_.emplace_back, Call(&class_ptr, func_ptrs, GET_HANDLE(clusterHandle_ + 1, func_ptrs)))
      GET_HANDLE(++clusterHandle_, POINTER_INT_CAST(nullptr));
    }

    template<typename ...Args>
    void Invoke(Args... args)
    {
      static std::map<Event *, std::mutex> m_mutex;
      std::lock_guard<std::mutex> lk(m_mutex[this]);
      for (auto &call : callList_)
        call.function(args...);
    }

    template<typename F>
    void Unhook(F func_ptr)
    VERIFY_TYPE(class_member_exclusion<F>())
    {
      RemoveCall(GET_HANDLE(POINTER_INT_CAST(nullptr), POINTER_INT_CAST(func_ptr)));
    }

    template<typename C, typename F>
    void Unhook(C &class_ptr, F func_ptr)
    VERIFY_TYPE(class_member_inclusion<C, F>())
    {
      RemoveCall(GET_HANDLE(POINTER_INT_CAST(&class_ptr), POINTER_INT_CAST(func_ptr)));
    }

    void Unhook(EVENT_HANDLE handle)
    {
      RemoveCall(handle);
    }

    void UnhookCluster(EVENT_HANDLE handle)
    {
      RemoveCluster(GET_CLUSTER(handle));
    }

    template<typename C>
    void UnhookClass(C &class_ptr)
    {
      static_assert(std::is_class_v<C>, "Class pointer provided not a pointer to a class");
      RemoveCluster(GET_HANDLE(POINTER_INT_CAST(&class_ptr), POINTER_INT_CAST(nullptr)));
    }

    template<typename ...Fs>
    void Unhook_fv(Fs ...func_ptrs)
    VERIFY_TYPE(class_member_exclusion<Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(Unhook, func_ptrs)
    }

    template<typename C, typename ...Fs>
    void Unhook_mv(C &class_ptr, Fs ...func_ptrs)
    VERIFY_TYPE(class_member_inclusion<C, Fs...>() && type_exclusion<EVENT_HANDLE, Fs...>())
    {
      PACK_EXPAND(Unhook, &class_ptr, func_ptrs)
    }

    size_t CallListSize() const
    {
        return callList_.size();
    }

  private:
    struct USet; struct CallHash;
    typedef typename std::conditional<KEEP_ORDER, std::vector<Call>, USet>::type CallListType;
    CallListType callList_;
    EVENT_HANDLE clusterHandle_ = 0;

    template<typename Invalid_Type, typename ...Types>
    static constexpr bool type_exclusion() // assures a type is not contained within Types...
    {
      static_assert(sizeof ...(Types) > 0, "Calling variadic function with no parameters");
      static_assert(!std::disjunction_v<std::is_same<Invalid_Type, Types>...>, "Invalid Type contained in Types variadic list");
      return true;
    }

    template<typename C, typename ...Fs>
    static constexpr bool class_member_inclusion() // assures Fs... are members of class C
    {
      static_assert(std::is_class_v<C>, "Provided class pointer is not a class");
      static_assert(sizeof ...(Fs) > 0, "Calling variadic function with no parameters");
      static_assert((... && is_member_function_of_v<C, Fs>), "A function in variadic list Fs... is not a non-static member of class C");
      return true;
    }

    template<typename ...Fs>
    static constexpr bool class_member_exclusion() // assures Fs... are not members of a class
    {
      static_assert(sizeof ...(Fs) > 0, "Calling variadic function with no parameters");
      static_assert(!std::disjunction_v<std::is_member_function_pointer<Fs>...>, "A function in variadic list Fs... is a class member");
      return true;
    }

    void RemoveCluster(EVENT_HANDLE cluster)
    {
      for (auto it = callList_.begin(); it != callList_.end();)
        if (cluster == GET_CLUSTER(it.handle))
          it = callList_.erase(it);
        else
          ++it;
    }

    void RemoveCall(EVENT_HANDLE handle)
    {
      auto call = std::find(callList_.begin(), callList_.end(), handle);
      if (call != callList_.end()) callList_.erase(call);
    }

    struct Call
    {
      Call()
      : function([](){}), handle(EVENT_HANDLE(0))
      {}

      template<typename F>
      Call(F func_ptr, EVENT_HANDLE handle)
      : function(func_ptr), handle(handle)
      {}

      template<typename C, typename F>
      Call(C class_ptr, F func_ptr, EVENT_HANDLE handle)
      : function(GetMethod(class_ptr, func_ptr)), handle(handle)
      {}

      template<typename C, typename R, typename ...Args>
      static auto GetMethod(C *class_ptr, R(C::*func_ptr)(Args...))
      {
        return [class_ptr, func_ptr](Args... args) { (void)(std::mem_fn(func_ptr)(class_ptr, args...)); };
      }

      bool operator==(const Call& other) const
      {
        return handle == other.handle;
      }

      operator EVENT_HANDLE()
      {
        return handle;
      }

      std::function<FUNCTION_SIGNATURE> function;
      EVENT_HANDLE handle;
    };

    struct CallHash
    {
      EVENT_HANDLE operator()(const Call& call) const
      {
        return call.handle;
      }
    };

    struct USet : public std::unordered_set<Call, CallHash>
    {
      USet() = default;
      template<typename ...Args>
      void emplace_back(Args... args)
      {
        assert(this->emplace(args...).second && "ERROR : Duplicate function hooked to event");
      }
    };

    //! Cast a member function pointer
    template<typename T>
    static constexpr std::uintptr_t POINTER_INT_CAST(T t)
    {
      return reinterpret_cast<std::uintptr_t>(*reinterpret_cast<void**>(&t));
    }

    template<typename, typename>
    struct is_member_function_of
    {
      static constexpr bool value = false;
    };

    #define DEF_IS_MEMBER_FUNCTION_OF(CV_REF_NOEXCEPT_OPT) \
      template<typename T, typename Ret, typename... Args> \
      struct is_member_function_of<T, Ret (T::*)(Args...) CV_REF_NOEXCEPT_OPT> \
      { \
          static constexpr bool value = true; \
      }

    #define DEF_IS_MEMBER_FUNCTION_OF_ELLIPSIS(CV_REF_NOEXCEPT_OPT) \
      template<typename T, typename Ret, typename... Args> \
      struct is_member_function_of<T, Ret (T::*)(Args..., ...) CV_REF_NOEXCEPT_OPT> \
      { \
          static constexpr bool value = true; \
      }

    #define PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_REF_OPT) \
      MACRO(CV_REF_OPT); \
      MACRO(CV_REF_OPT noexcept)

    #define PERMUTE_PMF_CV_REF(MACRO, CV_OPT) \
      PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT); \
      PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT &); \
      PERMUTE_PMF_CV_REF_NOEXCEPT(MACRO, CV_OPT &&)

    #define PERMUTE_PMF_CV(MACRO) \
      PERMUTE_PMF_CV_REF(MACRO, ); \
      PERMUTE_PMF_CV_REF(MACRO, const); \
      PERMUTE_PMF_CV_REF(MACRO, volatile); \
      PERMUTE_PMF_CV_REF(MACRO, const volatile)

    #define PERMUTE_PMF(MACRO) \
      PERMUTE_PMF_CV(MACRO); \
      PERMUTE_PMF_CV(MACRO##_ELLIPSIS)

    PERMUTE_PMF(DEF_IS_MEMBER_FUNCTION_OF);

    template<typename T, typename Fn>
    static inline constexpr bool is_member_function_of_v = is_member_function_of<T, Fn>::value;
};

#endif

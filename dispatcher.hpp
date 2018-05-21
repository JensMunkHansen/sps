/**
 * @file   dispatcher.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Sun Oct 29 20:38:37 2017
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 */

#pragma once

#include <sps/cenv.h>
#include <sps/eventif.hpp>
#include <sps/semaphore.hpp>
#include <sps/threadpool.hpp>
#include <sps/context.hpp>

#include <list>
#include <memory>
#include <future>

// EventHandler = Callback
// EventHandler is passed to EventListener
// EventListener is Signalled bu Event

namespace sps {

// TODO(JEM): Rename DispatherEvent to EventListener and construct this
//            using and event, which is created using a context

class IEventListener;

class DispatcherEvent: public IDispatcherEvent {
 private:
  Context* m_pContext;
  //  std::shared_ptr<IEventListener> m_pListener;   ///< Listener
  IEventListener* m_pListener;   ///< Listener - must survive dtor
  mutable std::mutex m_mutex;
  std::atomic<bool> m_valid{true};

 public:
  explicit inline DispatcherEvent(Context* pContext) {
    m_pContext = pContext;
    m_pListener = nullptr;
  }
  int ListenerAdd(IEventListener* pListener) SPS_OVERRIDE {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pListener) {
      return -1;
    } else if (m_valid) {
      m_pListener = pListener;
    }
    return 0;
  }
  inline bool Signal() SPS_OVERRIDE {
    // atomic m_valid
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_valid && m_pListener) {
      m_pListener->Signal(this);
      return true;
    }
    return false;
  }
  ~DispatcherEvent() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_valid = false;
  }
};

class EventListener : public IEventListener {
 public:
  class ICallBack {
   public:
    virtual ~ICallBack() = default;
    ICallBack(ICallBack&& other) = default;
    ICallBack& operator=(ICallBack&& other) = default;
    virtual void Execute() = 0;

   protected:
    ICallBack() = default;

   private:
    // Prevent copying
    ICallBack(const ICallBack& rhs) = delete;
    ICallBack& operator=(const ICallBack& rhs) = delete;
  };

  // EventHandler... add reference to event here
  template <class Func>
  class CallBack : public ICallBack {
   public:
    explicit CallBack(Func&& func) : m_func{std::move(func)} {}
    CallBack(CallBack&& other) = default;
    CallBack& operator=(CallBack&& other) = default;
    ~CallBack() SPS_OVERRIDE = default;
    void Execute() SPS_OVERRIDE { m_func(); }

   private:
   public:
    CallBack(const CallBack& rhs) = delete;
    CallBack& operator=(const CallBack& rhs) = delete;
    Func m_func;
  };

  mutable std::mutex m_mutex;
  mutable std::condition_variable m_condition;
  std::uint32_t m_id;
  bool m_bSignaled;
  bool m_bValid;

  // Replace with event
  Context* m_pContext;

 public:
  // Change to EventHandler = std::pair<list<id, status>, ICallBack>
  std::list<std::unique_ptr<ICallBack> > m_callbacks;

 public:
  explicit inline EventListener(Context* pContext) : m_bValid(true) {
    m_pContext = pContext;
  }

  EventListener(EventListener&& other) = default;
  EventListener& operator=(EventListener&& other) = default;
  ~EventListener() SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_bValid = false;
  };

  bool Signal(IDispatcherEvent* pEvent) SPS_OVERRIDE {
    bool bWasSignalled;
    m_mutex.lock();
    bWasSignalled = m_bSignaled;
    m_bSignaled = true;
    if (m_pContext && m_bValid) {
      for (auto& cb : m_callbacks) {
        // using ResultType =
        //    std::result_of_t<decltype(cb.get()->* &ICallBack::Execute)()>;
        // TODO(JEM): Make threadpool accept std::function
        m_pContext->ThreadPoolGet()->submit([&]()->void {
          cb.get()->Execute();
        });
      }
    }
    m_mutex.unlock();
    m_condition.notify_all();  // Notify completion if someone waits
    return bWasSignalled == false;
  }

  // When it only fires when event is complete, we call it an eventhandler
  template <typename Func, typename... Args>
  void RegisterCallback(Func&& func, Args&&... args) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_bValid) {
      auto boundTask = std::bind(std::forward<Func>(func),
                                 std::forward<Args>(args)...);

      using ResultType = std::result_of_t<decltype(boundTask)()>;

      using Function = std::function<ResultType()>;

      using CallBackType = CallBack<Function>;

      Function callback{std::move(boundTask)};

      // static_assert(std::is_same<ResultType, void>::value, "");
      m_callbacks.push_back(std::make_unique<CallBackType>(std::move(callback)));
    }
  }
};
}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */

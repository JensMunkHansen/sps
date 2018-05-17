/**
 * @file   eventif.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Mar  5 22:07:14 2018
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen

 */
#pragma once

#include <memory>

namespace sps {
class IEventListener;

class IDispatcherEvent {
 public:
  virtual ~IDispatcherEvent() = default;
  IDispatcherEvent(IDispatcherEvent&& other) = default;
  IDispatcherEvent& operator=(IDispatcherEvent&& other) = default;

  virtual inline bool Signal() = 0;
  virtual int ListenerAdd(IEventListener* pListener) = 0;
 protected:
  IDispatcherEvent() = default;
 private:
  // Prevent copying
  IDispatcherEvent(const IDispatcherEvent& rhs) = delete;
  IDispatcherEvent& operator=(const IDispatcherEvent& rhs) = delete;
};

class IEventListener  {
 public:
  static int Create(IEventListener** ppEvent);
  static int Destroy(IEventListener* pEvent);

  // Add method to register callback or eventhandler

  virtual ~IEventListener() = default;
  IEventListener(IEventListener&& other) = default;
  IEventListener& operator=(IEventListener&& other) = default;

  virtual bool Signal(IDispatcherEvent* pEvent) = 0;

 protected:
  IEventListener() = default;
 private:
  IEventListener(const IEventListener& rhs) = delete;
  IEventListener& operator=(const IEventListener& rhs) = delete;
};
}  // namespace sps

/**
 * @file   context_test.cpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Tue Feb 27 03:35:26 2018
 *
 * @brief
 *
 * Copyright 2018 Jens Munk Hansen
 */

#include <sps/context.hpp>

int main(int argc, char* argv[]) {
  SPS_UNREFERENCED_PARAMETERS(argc, argv);
  // Create context
  sps::ContextIF* pContext = nullptr;
  sps::Context::Create(&pContext);

  // Create Event
  // sps::EventIF* pEvent = nullptr;
  // sps::Event::Create(&pEvent, pContext);

  typedef int (*Callback)(void*);

  // RegCallback ( int (*fptr)(void*), status, userdata, event)

  // SetEventCallback(event, status, void (*fptr)(event, status, userdata), userdata)
  // Function will be called when status == status

  // DeferCall(fptr, size_t nWaitEvents, EventIF* evWait, EventIF* evComplete)
  // One thread finishes by enqueing new task or signalling an event

  return 0;
}

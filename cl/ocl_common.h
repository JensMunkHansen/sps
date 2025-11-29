#pragma once
#include <stdarg.h>

#ifdef __cplusplus
#define __STDC_FORMAT_MACROS 1 // Necessary for __cplusplus and -std=c++0x
#endif

#include <inttypes.h>

#include "CL/cl.h"

#include <errno.h>

#include <CL/cl.h>

/* Clean-up handler: Calls argument with prototype void (*)() if present */
#ifdef __GNUC__
#define NUMARGS(...) (sizeof((void (*[])()){ 0, ##__VA_ARGS__ }) / sizeof(void (*)()) - 1)
#define CLEANUP(...) cleanup(NUMARGS(__VA_ARGS__), ##__VA_ARGS__)
#else
#define NUMARGS(...) (sizeof((void (*[])()){ __VA_ARGS__ }) / sizeof(void (*)()))
#define CLEANUP(...) (cleanup(NUMARGS(__VA_ARGS__), __VA_ARGS__))
#endif

void cleanup(int numargs, ...)
{
  if (numargs == 0)
    return;

  void (*pCleanup)();

  va_list ap;
  va_start(ap, numargs);

  if (numargs--)
    pCleanup = va_arg(ap, void (*)());
  va_end(ap);

  pCleanup();

  return;
}

/*
   Calling macros for OpenCL functions using clerrno:

     CallClErr(function, (arguments))
     CallClErrExit(function, (arguments), return value)
     CallClErrExit(function, (arguments), return value, void (clean*)())

*/

/// Global errno variable
cl_int clerrno = CL_SUCCESS;

/**
 * strclerror
 *
 * @param status
 *
 * @return
 */
static const char* strclerror(cl_int status)
{
  static struct
  {
    cl_int code;
    const char* msg;
  } error_table[] = {
    { CL_SUCCESS, "success" },
    {
      CL_DEVICE_NOT_FOUND,
      "device not found",
    },
    {
      CL_DEVICE_NOT_AVAILABLE,
      "device not available",
    },
    {
      CL_COMPILER_NOT_AVAILABLE,
      "compiler not available",
    },
    {
      CL_MEM_OBJECT_ALLOCATION_FAILURE,
      "mem object allocation failure",
    },
    {
      CL_OUT_OF_RESOURCES,
      "out of resources",
    },
    {
      CL_OUT_OF_HOST_MEMORY,
      "out of host memory",
    },
    {
      CL_PROFILING_INFO_NOT_AVAILABLE,
      "profiling not available",
    },
    {
      CL_MEM_COPY_OVERLAP,
      "memcopy overlaps",
    },
    {
      CL_IMAGE_FORMAT_MISMATCH,
      "image format mismatch",
    },
    {
      CL_IMAGE_FORMAT_NOT_SUPPORTED,
      "image format not supported",
    },
    {
      CL_BUILD_PROGRAM_FAILURE,
      "build program failed",
    },
    {
      CL_MAP_FAILURE,
      "map failed",
    },
    {
      CL_INVALID_VALUE,
      "invalid value",
    },
    {
      CL_INVALID_DEVICE_TYPE,
      "invalid device type",
    },
    {
      CL_INVALID_PLATFORM,
      "invalid platform",
    },
    {
      CL_INVALID_DEVICE,
      "invalid device",
    },
    {
      CL_INVALID_CONTEXT,
      "invalid CONTEXT",
    },
    {
      CL_INVALID_QUEUE_PROPERTIES,
      "invalid QUEUE_PROPERTIES",
    },
    {
      CL_INVALID_COMMAND_QUEUE,
      "invalid COMMAND_QUEUE",
    },
    {
      CL_INVALID_HOST_PTR,
      "invalid HOST_PTR",
    },
    {
      CL_INVALID_MEM_OBJECT,
      "invalid MEM_OBJECT",
    },
    {
      CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
      "invalid IMAGE_FORMAT_DESCRIPTOR",
    },
    {
      CL_INVALID_IMAGE_SIZE,
      "invalid IMAGE_SIZE",
    },
    {
      CL_INVALID_SAMPLER,
      "invalid SAMPLER",
    },
    {
      CL_INVALID_BINARY,
      "invalid BINARY",
    },
    {
      CL_INVALID_BUILD_OPTIONS,
      "invalid BUILD_OPTIONS",
    },
    {
      CL_INVALID_PROGRAM,
      "invalid PROGRAM",
    },
    {
      CL_INVALID_PROGRAM_EXECUTABLE,
      "invalid PROGRAM_EXECUTABLE",
    },
    {
      CL_INVALID_KERNEL_NAME,
      "invalid KERNEL_NAME",
    },
    {
      CL_INVALID_KERNEL_DEFINITION,
      "invalid KERNEL_DEFINITION",
    },
    {
      CL_INVALID_KERNEL,
      "invalid KERNEL",
    },
    {
      CL_INVALID_ARG_INDEX,
      "invalid ARG_INDEX",
    },
    {
      CL_INVALID_ARG_VALUE,
      "invalid ARG_VALUE",
    },
    {
      CL_INVALID_ARG_SIZE,
      "invalid ARG_SIZE",
    },
    {
      CL_INVALID_KERNEL_ARGS,
      "invalid KERNEL_ARGS",
    },
    {
      CL_INVALID_WORK_DIMENSION,
      "invalid WORK_DIMENSION ",
    },
    {
      CL_INVALID_WORK_GROUP_SIZE,
      "invalid WORK_GROUP_SIZE",
    },
    {
      CL_INVALID_WORK_ITEM_SIZE,
      "invalid WORK_ITEM_SIZE",
    },
    {
      CL_INVALID_GLOBAL_OFFSET,
      "invalid GLOBAL_OFFSET",
    },
    {
      CL_INVALID_EVENT_WAIT_LIST,
      "invalid EVENT_WAIT_LIST",
    },
    {
      CL_INVALID_EVENT,
      "invalid EVENT",
    },
    {
      CL_INVALID_OPERATION,
      "invalid OPERATION",
    },
    {
      CL_INVALID_GL_OBJECT,
      "invalid GL_OBJECT",
    },
    {
      CL_INVALID_BUFFER_SIZE,
      "invalid BUFFER_SIZE",
    },
    {
      CL_INVALID_MIP_LEVEL,
      "invalid MIP_LEVEL",
    },
    {
      CL_INVALID_GLOBAL_WORK_SIZE,
      "invalid GLOBAL_WORK_SIZE",
    },
    { 0, NULL },
  };

  static char unknown[25];
  size_t i;

  for (i = 0; error_table[i].msg != NULL; i++)
  {
    if (error_table[i].code == status)
    {
      return error_table[i].msg;
    }
  }

  snprintf(unknown, sizeof(unknown), "unknown error %d", status);
  return unknown;
}

#define CallClErr(fun, arg)                                                                        \
  {                                                                                                \
    intptr_t clerr64 = (intptr_t)(fun arg);                                                        \
    cl_int clerr = (cl_int)clerr64;                                                                \
    if ((clerr == 0) && (clerrno != CL_SUCCESS))                                                   \
    {                                                                                              \
      FailClErr(#fun, clerrno)                                                                     \
    }                                                                                              \
    else if ((clerr != CL_SUCCESS) && (clerrno != CL_SUCCESS))                                     \
      FailClErr(#fun, clerr) clerrno = CL_SUCCESS;                                                 \
  }

#define CallClErrExit(fun, arg, ret, ...)                                                          \
  {                                                                                                \
    intptr_t clerr64 = (intptr_t)(fun arg);                                                        \
    cl_int clerr = (cl_int)clerr64;                                                                \
    if ((clerr == 0) && (clerrno != CL_SUCCESS))                                                   \
    {                                                                                              \
      FailClErr(#fun, clerrno) CLEANUP(__VA_ARGS__);                                               \
      return ret;                                                                                  \
    }                                                                                              \
    else if ((clerr != CL_SUCCESS) && (clerrno != CL_SUCCESS))                                     \
    {                                                                                              \
      FailClErr(#fun, clerr) CLEANUP(__VA_ARGS__);                                                 \
      return ret;                                                                                  \
    }                                                                                              \
    clerrno = CL_SUCCESS;                                                                          \
  }

#define FailClErr(msg, clerrno)                                                                    \
  {                                                                                                \
    (void)fprintf(stderr, "FAILED: %s %d %s(clerrno=%d strclerror=%s)\n", __FILE__, __LINE__, msg, \
      clerrno, strclerror(clerrno));                                                               \
    (void)fflush(stderr);                                                                          \
  }

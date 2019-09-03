# TODO(JMH): Remove dependence of add_gtest_dependencies

cmake_minimum_required(VERSION 3.0)

include(cmake/utils.cmake)
include(cmake/simd.cmake)
include(cmake/gtest_macros.cmake)
include(GenerateExportHeader)

set(CMAKE_DEBUG_POSTFIX "")

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  )

option (SPS_SHARED_LIBS "Build Shared Libraries" OFF)
set(SPS_LIB_TYPE STATIC)
if (SPS_SHARED_LIBS)
  set(SPS_LIB_TYPE SHARED)
endif()

include(CheckIncludeFile)
include(CheckSymbolExists)
#include(ValidateHeaders)

check_symbol_exists(itoa stdlib.h HAVE_ITOA)
check_symbol_exists(utoa stdlib.h HAVE_UTOA)
check_symbol_exists(ptoa stdlib.h HAVE_PTOA)

option (SPS_Signals "Include signal processing" ON)

option (BUILD_SPS_TEST "Build SPS tests" OFF)

check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(mqueue.h HAVE_MQUEUE_H)
check_include_file(signal.h HAVE_SIGNAL_H)
check_include_file(pthread.h HAVE_PTHREAD_H)

# TODO(JMH): Create a sps/sps directory structure
#            Right now the build directory must be named sps
if (${PROJECT_NAME} STREQUAL "Project")
  include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}/..")  # to find sps/config.h
  include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/..")  # to find sps/xxx.h
else()
  include_directories(
    "${PROJECT_BINARY_DIR}")  # to find sps/config.h
  include_directories(
    "${PROJECT_SOURCE_DIR}")  # to find sps/xxx.h
endif()

find_package(FFTW REQUIRED)
if (WIN32)
  # TODO: Support non-default install locations on *NIX
  include_directories(${FFTW_INCLUDES})
endif()

# Rotation convention:
set (RotationConvention "yxy" CACHE STRING "Angle convention used for angular arguments")
set(RotationConventionValues "zyz;yxy;xyz")
set_property(CACHE RotationConvention PROPERTY STRINGS ${RotationConventionValues})

if (${RotationConvention} STREQUAL "xyz")
  # Extrinsic (i.e. non-rotating)
  set(ROTATION_CONVENTION "2")
elseif("${RotationConvention}" STREQUAL "yxy")
  # Intrinsic (i.e. rotating)
  set(ROTATION_CONVENTION "1")
else() # Convention used is zyz
  # Intrinsic (i.e. rotating)
  set(ROTATION_CONVENTION "0")
endif()

set (USE_FFTW_THREADS 0)
option (USE_FFTWThreads "Use threading for FFTW" OFF)
if (USE_FFTWThreads)
  set(USE_FFTW_THREADS 1)
endif()

# Enable config.h
set(HAVE_CONFIG_H 1)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

#use_cxx11()

if (CYGWIN)
  message("CYGWIN")
  set(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS} -mfma -msse2 -msse3 -msse4 -mavx -pedantic -Wall")
endif()

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd6326")
  add_definitions("/wd6385")
endif()

# Squeezed in for clang (remove this)
if (UNIX)
  set(CMAKE_CXX_FLAGS " -std=c++17 -pedantic -Wall ${CMAKE_CXX_FLAGS} -std=c++17") # Was c++14
  if (ENABLE_SSE2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
  endif()
  if (ENABLE_SSE3)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
  endif()
  if (ENABLE_SSE4)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4 -msse4.1 -msse4.2")
  endif()
  if (ENABLE_AVX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
  endif()
  if (ENABLE_FMA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfma")
  endif()
  if (ENABLE_AVX2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
  endif()
endif()

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")

  if (ENABLE_AVX)
    add_compile_flags("C;CXX" "/arch:AVX")
    add_definitions("/arch:AVX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
  endif()
  if (ENABLE_AVX2)
    add_definitions("/arch:AVX2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
  endif()
endif()

set(sps_HEADERS
  debug.h
  stdio.h
  stdlib.h
  cenv.h
  cerr.h
  math.h
  malloc.h
  mm_malloc.h
  zip
  functional.hpp
  smath_types.hpp
  smath.hpp
  sps_threads.hpp
  multi_malloc.hpp
  aligned_array.hpp
  memory
  threadpool.hpp
  indexed_types.hpp
  context.hpp
  contextif.hpp
  resource.hpp
  win32/memory
  unix/memory
)

set(sps_SOURCES
  smath.cpp
  context.cpp
  resource.cpp
  threadpool.cpp
  # smath_types.cpp - clean up (it is obsolete as is)
)

if (USE_PROGRESS_BAR)
  # Must be defined for stand-alone sps library
  set(sps_HEADERS "${sps_HEADERS}" "progress.hpp" "progress_if.hpp")
  set(sps_SOURCES "${sps_SOURCES}" "progress.cpp")
endif()

if (SPS_Signals)
  # Must be defined for stand-alone sps library
  set(sps_HEADERS "${sps_HEADERS}" "msignals.hpp")
  set(sps_SOURCES "${sps_SOURCES}" "msignals.cpp")
endif()

if (HAVE_MQUEUE_H)
  set(sps_HEADERS "${sps_HEADERS}" "sps_mqueue.hpp")
  set(sps_SOURCES "${sps_SOURCES}" "sps_mqueue.cpp")
endif()

if (UNIX AND NOT WIN32 AND NOT CYGWIN)
  set(sps_SOURCES "${sps_SOURCES}" "strace.cpp")
endif()

add_library(sps ${SPS_LIB_TYPE} ${sps_HEADERS} ${sps_SOURCES})

if (0) #UNIX AND NOT WIN32 AND NOT CYGWIN)
  set(bum)
  validate_headers(
    bum
    TARGET sps
    CXX_FLAGS ${CMAKE_CXX_FLAGS}
    CFLAGS    ${CMAKE_CFLAGS}
    INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/.."
    HEADERS ${sps_HEADERS})
endif()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set_property(TARGET sps PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if (SPS_Signals)
  if (WIN32)
    link_directories(${FFTW_INCLUDES})
    # Is this necessary???
    if (BUILD_SHARED_LIBS)
      target_link_libraries(sps ${FFTW_LIBRARIES})
    else()
      link_libraries(sps ${FFTW_LIBRARIES})
    endif()
  else()
    link_libraries(sps ${FFTW_LIBRARIES})
  endif()
endif()

generate_export_header(sps)# DEFINE_NO_DEPRECATED)

if (BUILD_SPS_TEST)
  if (WIN32)
    set(GTEST_ROOT "C:/Program Files/googletest-distribution")
  endif()

  find_package( GTest REQUIRED )

  # TODO: Support using pkg-config
  if (0)
    find_package(PkgConfig)
    pkg_search_module(GTEST REQUIRED gtest)
    pkg_check_modules(GTEST REQUIRED gtest>=1.7.0)

    message("libraries: ${GTEST_LIBRARIES}")
    message("library dirs: ${GTEST_LIBRARY_DIRS}")
    message("LDFLAGS: ${GTEST_LDFLAGS}")

    include_directories(
      ${GTEST_INCLUDE_DIRS}
    )

    # Similarly for gmock : ${GMOCK_INCLUDE_DIRS}
  endif()

  find_package( Threads REQUIRED )

  include_directories(${GTEST_INCLUDE_DIRS})

  if (1)
    add_executable(indexed_types_test indexed_types_test.cpp)
    add_gtest_dependencies(indexed_types_test)
  endif()
  add_executable(extintrin_test extintrin_test.cpp)
  add_gtest_dependencies(extintrin_test)

  add_executable(trigintrin_test trigintrin_test.cpp)
  add_gtest_dependencies(trigintrin_test)

  add_executable(smath_test smath_test.cpp smath.cpp)
  add_gtest_dependencies(smath_test)

  add_executable(memory_test memory_test.cpp)
  target_link_libraries(memory_test ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

  add_executable(multi_malloc_test multi_malloc_test.cpp)
  add_gtest_dependencies(multi_malloc_test)

  add_executable(aligned_array_test aligned_array_test.cpp)
  add_gtest_dependencies(aligned_array_test)

  add_executable(semaphore_test semaphore_test.cpp)
  add_gtest_dependencies(semaphore_test)

  add_executable(threadpool_test threadpool_test.cpp threadpool.cpp)
  add_gtest_dependencies(threadpool_test)

  add_executable(thread_test thread_test.cpp)
  add_gtest_dependencies(thread_test)

  add_executable(globals_test globals_test.cpp)
  add_gtest_dependencies(globals_test)

  add_executable(bitswitch_test bitswitch_test.cpp)
  set_target_properties(bitswitch_test PROPERTIES
    CXX_STANDARD 17)    
  add_gtest_dependencies(bitswitch_test)

  add_executable(allocator_test allocator_test.cpp)
  add_gtest_dependencies(allocator_test)

  if (MSVC)
    set_source_files_properties(allocator_test.cpp PROPERTIES COMPILE_FLAGS "/wd4324")
  endif()
  
  add_executable(default_test default_test.cpp)
  add_gtest_dependencies(default_test)

  add_executable(aligned_allocator_test aligned_allocator_test.cpp)
  add_gtest_dependencies(aligned_allocator_test)

  add_executable(macro_test macro_test.cpp)
  add_gtest_dependencies(macro_test)

  if (UNIX)
    add_executable(weak_binder_test weak_binder_test.cpp)
    set_target_properties(weak_binder_test PROPERTIES
      CXX_STANDARD 17)    
    add_gtest_dependencies(weak_binder_test)
  endif()
  set(sgc_test_SOURCES
    sgc.cpp
    )
  if (UNIX AND NOT WIN32 AND NOT CYGWIN)
    set(sgc_test_SOURCES "${sgc_test_SOURCES}" "strace.cpp")
  endif()

  add_executable(sgc_test ${sgc_test_SOURCES})
  add_gtest_dependencies(sgc_test)
  if (UNIX AND NOT WIN32 AND NOT CYGWIN)
    target_link_libraries(sgc_test dl)
  endif()

  set(mimo_test_SOURCES
    mimo_test.cpp
    )
  if (UNIX AND NOT WIN32 AND NOT CYGWIN)
    set(mimo_test_SOURCES "${mimo_test_SOURCES}" "strace.cpp")
  endif()

  add_executable(mimo_test ${mimo_test_SOURCES})
  add_gtest_dependencies(mimo_test)
  if (UNIX AND NOT WIN32 AND NOT CYGWIN)
    target_link_libraries(mimo_test dl)
  endif()

  add_executable(dispatcher_test resource.cpp context.cpp dispatcher_test.cpp)
  add_gtest_dependencies(dispatcher_test)
  if (UNIX)
    target_link_libraries(dispatcher_test dl)
  endif()

  add_executable(context_test resource.cpp context.cpp context_test.cpp)
  add_gtest_dependencies(context_test)
  if (UNIX)
    target_link_libraries(context_test dl)
  endif()

  if (SPS_Signals)
    add_executable(signals_test signals_test.cpp msignals.cpp)
    add_gtest_dependencies(signals_test)
  endif()

  # Enable the tests
  enable_testing()
  if (UNIX)
    add_test(NAME    indexed_types_test
             COMMAND indexed_types_test)
  endif()
  add_test(NAME    extintrin_test
           COMMAND extintrin_test)

  add_test(NAME    trigintrin_test
           COMMAND trigintrin_test)

  add_test(NAME    smath_test
           COMMAND smath_test)

  add_test(NAME    memory_test
           COMMAND memory_test)

  add_test(NAME    multi_malloc_test
           COMMAND multi_malloc_test)

  add_test(NAME    aligned_array_test
           COMMAND aligned_array_test)

  add_test(NAME    semaphore_test
           COMMAND semaphore_test)

  add_test(NAME    threadpool_test
           COMMAND threadpool_test)

  add_test(NAME    context_test
           COMMAND context_test)

  add_test(NAME    thread_test
           COMMAND thread_test)

  add_test(NAME    mimo_test
           COMMAND mimo_test)

  add_test(NAME    globals_test
           COMMAND globals_test)

  add_test(NAME    bitswitch_test
           COMMAND bitswitch_test)

  add_test(NAME    default_test
           COMMAND default_test)

  add_test(NAME    aligned_allocator_test
           COMMAND aligned_allocator_test)

  add_test(NAME    macro_test
           COMMAND macro_test)

  if (SPS_Signals)
    add_test(NAME    signals_test
             COMMAND signals_test)
  endif()

  # Link to threading library (general)
  set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
endif()

# SWIG Target

if (SPS_Signals)
  # Need signals.cpp and link to FFTW

  # Figure out why cstdarg => FILE__ undefined
  if (MSVC)
    add_definitions(-DSWIG_PYTHON_INTERPRETER_NO_DEBUG)
  endif()

  find_package(SWIG REQUIRED)
  include(${SWIG_USE_FILE})

  if (ENABLE_PYTHON3)
    set(Python_ADDITIONAL_VERSIONS 3.5)
    find_package(PythonInterp 3 REQUIRED)
    find_package(PythonLibs)
  else()
    set(Python_ADDITIONAL_VERSIONS 2.7)
    # HACK
    if (WIN32)
      find_package(PythonInterp 2.7 REQUIRED)
    else()
      find_package(PythonInterp 2.7 REQUIRED)
      #find_package(PythonInterp 2.7.9 EXACT REQUIRED)
    endif()
    find_package(PythonLibs 2.7 REQUIRED)
  endif()

  include_directories(${PYTHON_INCLUDE_PATH})

  if (MSVC)
    set(CMAKE_SWIG_FLAGS "-D_SWIG_WIN32")
  endif()

  set_source_files_properties(swig_signals.i PROPERTIES CPLUSPLUS ON)

  set(swig_signals_HEADERS
    msignals.hpp
    )

  set(swig_signals_SOURCES
    )

if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
  swig_add_module(swig_signals python swig_signals.i ${swig_signals_HEADERS} ${swig_signals_SOURCES})
else()
  swig_add_library(swig_signals LANGUAGE python SOURCES swig_signals.i ${swig_signals_HEADERS} ${swig_signals_SOURCES})
endif()

  if (MSVC)
    # Potential uninitialized variable in SWIG_AsVal_
    set_source_files_properties( ${swig_generated_file_fullname} PROPERTIES COMPILE_FLAGS "/wd6246 /wd6244 /wd6011 /wd4459 /wd4701 /wd4127 /wd4456")

    # Allow to debug under windows, if debug versions of Python are missing
    string(REPLACE "_d" "" PYTHON_LIBRARIES "${PYTHON_LIBRARIES}")
  endif()

  swig_link_libraries(swig_signals sps ${FFTW_LIBRARIES} ${PYTHON_LIBRARIES})

  if (WIN32)
    # pyconfig.h is not autogenerated on Windows. To avoid warnings, we
    # add a compiler directive
    get_directory_property(DirDefs COMPILE_DEFINITIONS )
    set_target_properties(_swig_signals PROPERTIES
      COMPILE_DEFINITIONS "${DirDefs};HAVE_ROUND")
  endif()

endif()

# TODO: Create an "export-set" for SPS and import this in FNM and SOFUS using
#       find_package or include(sps.cmake)

if( ${PROJECT_NAME} STREQUAL "BFTX")
  install(TARGETS sps
    # IMPORTANT: Add the fnm library to the "export-set"
    EXPORT BftXTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
    ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/sps"
    COMPONENT dev)
endif()

if( ${PROJECT_NAME} STREQUAL "fnm")
  install(TARGETS sps
    # IMPORTANT: Add the fnm library to the "export-set"
    EXPORT SofusTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
    ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/sps"
    COMPONENT dev)
endif()

if( ${PROJECT_NAME} STREQUAL "Sofus")
  install(TARGETS sps
    # IMPORTANT: Add the fnm library to the "export-set"
    EXPORT SofusTargets
    RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
    ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
    LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/sps"
    COMPONENT dev)
endif()

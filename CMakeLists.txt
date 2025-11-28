cmake_minimum_required(VERSION 3.20)

# Support standalone build
if(NOT DEFINED PROJECT_NAME)
  project(sps C CXX)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../CMake)
  include(spsInstallDirs)

  # Create build interface target for standalone build
  add_library(build INTERFACE)
  add_library(sps::build ALIAS build)
  target_compile_features(build INTERFACE cxx_std_17)
  include(spsCompilerPlatformFlags)
  include(spsCompilerWarningFlags)

  # SIMD detection for standalone build
  include(spsSIMD)

  # Enable PIC for all targets (needed for Python bindings)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(NOT DEFINED ENABLE_PYTHON3)
  set(ENABLE_PYTHON3 ON)
endif()

# === Options ===
option(SPS_SHARED_LIBS "Build Shared Libraries" OFF)
option(SPS_STRACE "Use strace" OFF)
option(SPS_Signals "Include signal processing" ON)
option(BUILD_SPS_TEST "Build SPS tests" ON)

set(SPS_LIB_TYPE STATIC)
if(SPS_SHARED_LIBS)
  set(SPS_LIB_TYPE SHARED)
endif()

set(STRACE 0)
if(SPS_STRACE)
  set(STRACE 1)
endif()

# === Feature detection ===
include(CheckIncludeFile)
include(CheckSymbolExists)
include(GenerateExportHeader)

check_symbol_exists(itoa stdlib.h HAVE_ITOA)
check_symbol_exists(utoa stdlib.h HAVE_UTOA)
check_symbol_exists(ptoa stdlib.h HAVE_PTOA)

check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(mqueue.h HAVE_MQUEUE_H)
check_include_file(signal.h HAVE_SIGNAL_H)
check_include_file(pthread.h HAVE_PTHREAD_H)

# === Dependencies ===
find_package(FFTW REQUIRED COMPONENTS FLOAT_LIB DOUBLE_LIB)

# Rotation convention
set(RotationConvention "yxy" CACHE STRING "Angle convention used for angular arguments")
set(RotationConventionValues "zyz;yxy;xyz")
set_property(CACHE RotationConvention PROPERTY STRINGS ${RotationConventionValues})

if(${RotationConvention} STREQUAL "xyz")
  set(ROTATION_CONVENTION "2")
elseif("${RotationConvention}" STREQUAL "yxy")
  set(ROTATION_CONVENTION "1")
else()
  set(ROTATION_CONVENTION "0")
endif()

set(USE_FFTW_THREADS 0)
option(USE_FFTWThreads "Use threading for FFTW" OFF)
if(USE_FFTWThreads)
  set(USE_FFTW_THREADS 1)
endif()

# Enable config.h
set(HAVE_CONFIG_H 1)

# Configure config.h (standalone puts it in sps/ subdir, parent project handles location)
if(NOT PROJECT_NAME STREQUAL "sps")
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    @ONLY)
endif()

# === Source files ===
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
  smath_matrix_type.hpp
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
)

if(WIN32)
  list(APPEND sps_HEADERS win32/memory)
endif()

if(USE_PROGRESS_BAR)
  list(APPEND sps_HEADERS progress.hpp progress_if.hpp)
  list(APPEND sps_SOURCES progress.cpp)
endif()

if(SPS_Signals)
  list(APPEND sps_HEADERS msignals.hpp)
  list(APPEND sps_SOURCES msignals.cpp)
endif()

if(HAVE_MQUEUE_H)
  list(APPEND sps_HEADERS sps_mqueue.hpp)
  list(APPEND sps_SOURCES sps_mqueue.cpp)
endif()

# === Library target ===
add_library(sps ${SPS_LIB_TYPE} ${sps_HEADERS} ${sps_SOURCES})

# Include directories depend on whether we're standalone or part of parent project
if(PROJECT_NAME STREQUAL "sps")
  # Standalone: create sps/ subdirectory structure in build dir for generated headers
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sps)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/sps/config.h"
    @ONLY)
  # Generate export header in sps/ subdir for standalone build
  generate_export_header(sps
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/sps/sps_export.h")
  target_include_directories(sps PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>)
else()
  # Parent project handles include paths
  generate_export_header(sps)
  target_include_directories(sps PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
    $<INSTALL_INTERFACE:include>)
endif()

# Link to build interface for compiler flags
if(TARGET build)
  target_link_libraries(sps PRIVATE $<BUILD_INTERFACE:build>)
endif()

# Link FFTW
if(SPS_Signals)
  target_include_directories(sps PRIVATE ${FFTW_INCLUDES})
  target_link_libraries(sps PUBLIC ${FFTW_LIBRARIES})
endif()

# === Tests ===
if(BUILD_SPS_TEST)
  include(spsGTest)

  # Set include directories based on standalone vs parent project
  if(PROJECT_NAME STREQUAL "sps")
    set(_SPS_TEST_INCLUDE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/..
      ${CMAKE_CURRENT_BINARY_DIR})
  else()
    set(_SPS_TEST_INCLUDE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/..
      ${CMAKE_CURRENT_BINARY_DIR}/..)
  endif()

  sps_add_gtest(indexed_types_test indexed_types_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(extintrin_test extintrin_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(trigintrin_test trigintrin_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(smath_test smath_test.cpp smath.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(memory_test memory_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(multi_malloc_test multi_malloc_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(aligned_array_test aligned_array_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(align_test align_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(semaphore_test semaphore_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(threadpool_test threadpool_test.cpp threadpool.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(thread_test thread_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(globals_test globals_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})

  sps_add_gtest(bitswitch_test bitswitch_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  set_target_properties(bitswitch_test PROPERTIES CXX_STANDARD 14)

  sps_add_gtest(allocator_test allocator_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  if(MSVC)
    target_compile_options(allocator_test PRIVATE /wd4324)
  endif()

  sps_add_gtest(aligned_allocator_test aligned_allocator_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(macro_test macro_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(string_test string_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})

  if(UNIX)
    sps_add_gtest(weak_binder_test weak_binder_test.cpp
      INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
    set_target_properties(weak_binder_test PROPERTIES CXX_STANDARD 17)

    if(LINUX)
      sps_add_gtest(functional_test functional_test.cpp
        INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
    endif()
  endif()

  sps_add_gtest(sgc_test sgc.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(mimo_test mimo_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(dispatcher_test resource.cpp context.cpp dispatcher_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(context_test resource.cpp context.cpp context_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})
  sps_add_gtest(siso_test siso_test.cpp
    INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS})

  if(SPS_Signals)
    sps_add_gtest(signals_test signals_test.cpp msignals.cpp
      INCLUDE_DIRS ${_SPS_TEST_INCLUDE_DIRS}
      LIBRARIES ${FFTW_LIBRARIES})
    target_include_directories(signals_test PRIVATE ${FFTW_INCLUDES})
  endif()

  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS string_test)
endif()

# === SWIG Python bindings ===
if(SPS_Signals)
  if(MSVC AND _DEBUG_USING_PYTHON_RELEASE_RUNTIME)
    add_definitions(-DSWIG_PYTHON_INTERPRETER_NO_DEBUG)
  endif()

  find_package(SWIG)
  if(SWIG_FOUND)
    include(${SWIG_USE_FILE})

    if(WIN32)
      find_package(Python3 COMPONENTS Interpreter Development NumPy)
    else()
      find_package(Python3 3.9 COMPONENTS Interpreter Development NumPy)
    endif()

    if(Python3_FOUND)
      if(MSVC)
        set(CMAKE_SWIG_FLAGS "-D_SWIG_WIN32")
      endif()

      set_property(SOURCE swig_signals.i PROPERTY SWIG_MODULE_NAME swig_signals)
      set_source_files_properties(swig_signals.i PROPERTIES CPLUSPLUS ON)
      set_property(SOURCE swig_signals.i PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/..")

      set(swig_signals_HEADERS msignals.hpp)
      set(swig_signals_SOURCES)

      swig_add_library(swig_signals
        LANGUAGE python
        SOURCES swig_signals.i ${swig_signals_HEADERS} ${swig_signals_SOURCES})

      # Include directories depend on whether we're standalone or part of parent project
      if(PROJECT_NAME STREQUAL "sps")
        target_include_directories(swig_signals PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}/..
          ${CMAKE_CURRENT_BINARY_DIR}
          ${Python3_INCLUDE_DIRS}
          ${Python3_NumPy_INCLUDE_DIRS})
      else()
        target_include_directories(swig_signals PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}/..
          ${CMAKE_CURRENT_BINARY_DIR}/..
          ${Python3_INCLUDE_DIRS}
          ${Python3_NumPy_INCLUDE_DIRS})
      endif()

      if(MSVC)
        set_source_files_properties(${swig_generated_file_fullname}
          PROPERTIES COMPILE_FLAGS "/wd6246 /wd6244 /wd6011 /wd4459 /wd4701 /wd4706 /wd4127 /wd4456")
        if(_DEBUG_USING_PYTHON_RELEASE_RUNTIME)
          string(REPLACE "_d" "" Python3_LIBRARIES "${Python3_LIBRARIES}")
        endif()
      else()
        # Suppress warnings in SWIG-generated code for GCC/Clang
        set_source_files_properties(${swig_generated_file_fullname}
          PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast -Wno-unused-parameter -Wno-shadow")
      endif()

      swig_link_libraries(swig_signals sps ${FFTW_LIBRARIES} ${Python3_LIBRARIES})

      if(WIN32)
        get_directory_property(DirDefs COMPILE_DEFINITIONS)
        set_target_properties(${SWIG_MODULE_swig_signals_REAL_NAME} PROPERTIES
          COMPILE_DEFINITIONS "${DirDefs};HAVE_ROUND")
      endif()
    endif()
  endif()
endif()

# === Installation ===
if(${PROJECT_NAME} STREQUAL "BFTX")
  install(TARGETS sps
    EXPORT BftXTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sps" COMPONENT dev)
elseif(${PROJECT_NAME} STREQUAL "Sofus")
  # Install to both FnmTargets and SofusTargets export sets
  install(TARGETS sps
    EXPORT FnmTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sps" COMPONENT dev)
  install(TARGETS sps
    EXPORT SofusTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sps" COMPONENT dev)
endif()

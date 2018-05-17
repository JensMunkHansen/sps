include (CMakeParseArguments)

# validate_headers()
#
# Usage:
#   validate_headers(
#     ReturnValue
#     TARGET MyGreatProject
#     CXX_FLAGS
#     CFLAGS
#     INCLUDE_PATH
#   )
# where TARGET is used for getting compile options (if any)
#
# You can specify resource strings in arguments:
#   TARGET             - name of target from which to extract compile options
#   CXX_FLAGS          - needed to validate C++ headers
#   CFLAGS             - needed to validate C headers
#   INCLUDE_PATH       - needed to resolve any includes
#   HEADERS            - for testing
function(validate_headers returnvalue)
  set (options)
  set (oneValueArgs
    TARGET)
  set (multiValueArgs
    CXX_FLAGS
    CFLAGS
    INCLUDE_PATH
    HEADERS)
  cmake_parse_arguments(PRODUCT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # TODO: Use actual flags and figure out if C or C++
  if (NOT PRODUCT_CXX_FLAGS OR "${PRODUCT_CXX_FLAGS}" STREQUAL "")
    set(PRODUCT_CXX_FLAGS "-w -std=c++14 -fsyntax-only")
  endif()

  set(include_arguments)
  foreach( dir ${PRODUCT_INCLUDE_PATH} )
    set( include_arguments "${include_arguments} -I${dir}" )
  endforeach(dir)

  separate_arguments(include_arguments UNIX_COMMAND "${include_arguments}")

  foreach(header ${PRODUCT_HEADERS})
    add_custom_command(TARGET ${PRODUCT_TARGET}
      PRE_BUILD
      COMMAND "g++"
      -w -std=c++14
      ${include_arguments}
      -fsyntax-only "${CMAKE_CURRENT_SOURCE_DIR}/${header}")
  endforeach()
  set (${returnvalue} TRUE PARENT_SCOPE)
endfunction()

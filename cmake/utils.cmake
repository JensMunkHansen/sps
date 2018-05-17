macro(use_cxx11)
  if(CMAKE_VERSION VERSION_LESS "3.1")
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_C_FLAGS}")
    endif()
  else()
    set(CMAKE_CXX_STANDARD 11)
  endif()
endmacro(use_cxx11)

# target_compile_features(foobar PRIVATE cxx_range_for)

macro(add_compile_flags langs)
    foreach(_lang ${langs})
        string (REPLACE ";" " " _flags "${ARGN}")
        set ("CMAKE_${_lang}_FLAGS" "${CMAKE_${_lang}_FLAGS} ${_flags}")
        unset (${_lang})
        unset (${_flags})
    endforeach()
endmacro(add_compile_flags)

macro(set_source_files_compile_flags)
    foreach(file ${ARGN})
        get_filename_component(_file_ext ${file} EXT)
        set(_lang "")
        if ("${_file_ext}" STREQUAL ".m")
            set(_lang OBJC)
            # CMake believes that Objective C is a flavor of C++, not C,
            # and uses g++ compiler for .m files.
            # LANGUAGE property forces CMake to use CC for ${file}
            set_source_files_properties(${file} PROPERTIES LANGUAGE C)
        elseif("${_file_ext}" STREQUAL ".mm")
            set(_lang OBJCXX)
        endif()

        if (_lang)
            get_source_file_property(_flags ${file} COMPILE_FLAGS)
            if ("${_flags}" STREQUAL "NOTFOUND")
                set(_flags "${CMAKE_${_lang}_FLAGS}")
            else()
                set(_flags "${_flags} ${CMAKE_${_lang}_FLAGS}")
            endif()
            # message(STATUS "Set (${file} ${_flags}")
            set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS
                "${_flags}")
        endif()
    endforeach()
    unset(_file_ext)
    unset(_lang)
endmacro(set_source_files_compile_flags)

# A helper function to compile *.lua source into *.lua.c sources
function(lua_source varname filename)
    set (srcfile "${CMAKE_CURRENT_SOURCE_DIR}/${filename}")
    set (tmpfile "${CMAKE_CURRENT_BINARY_DIR}/${filename}.new.c")
    set (dstfile "${CMAKE_CURRENT_BINARY_DIR}/${filename}.c")
    get_filename_component(module ${filename} NAME_WE)
    get_filename_component(_name ${dstfile} NAME)
    string(REGEX REPLACE "${_name}$" "" dstdir ${dstfile})
    if (IS_DIRECTORY ${dstdir})
    else()
        file(MAKE_DIRECTORY ${dstdir})
    endif()

    ADD_CUSTOM_COMMAND(OUTPUT ${dstfile}
        COMMAND ${ECHO} 'const char ${module}_lua[] =' > ${tmpfile}
        COMMAND ${CMAKE_BINARY_DIR}/extra/txt2c ${srcfile} >> ${tmpfile}
        COMMAND ${ECHO} '\;' >> ${tmpfile}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${tmpfile} ${dstfile}
        COMMAND ${CMAKE_COMMAND} -E remove ${tmpfile}
        DEPENDS ${srcfile} txt2c)

    set(var ${${varname}})
    set(${varname} ${var} ${dstfile} PARENT_SCOPE)
endfunction()

function(bin_source varname srcfile dstfile)
    set(var ${${varname}})
    set(${varname} ${var} ${dstfile} PARENT_SCOPE)
    set (srcfile "${CMAKE_CURRENT_SOURCE_DIR}/${srcfile}")
    set (dstfile "${CMAKE_CURRENT_SOURCE_DIR}/${dstfile}")
    set (tmpfile "${dstfile}.tmp")
    get_filename_component(module ${dstfile} NAME_WE)

    ADD_CUSTOM_COMMAND(OUTPUT ${dstfile}
        COMMAND ${ECHO} 'const unsigned char ${module}_bin[] = {' > ${tmpfile}
        COMMAND ${CMAKE_BINARY_DIR}/extra/bin2c "${srcfile}" >> ${tmpfile}
        COMMAND ${ECHO} '}\;' >> ${tmpfile}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${tmpfile} ${dstfile}
        COMMAND ${CMAKE_COMMAND} -E remove ${tmpfile}
        DEPENDS ${srcfile} bin2c)
endfunction()

# A helper function to extract public API
function(apigen)
    set (dstfile "${CMAKE_CURRENT_BINARY_DIR}/tarantool.h")
    set (tmpfile "${dstfile}.new")
    set (headers)
    # Get absolute path for header files (required of out-of-source build)
    foreach (header ${ARGN})
        if (IS_ABSOLUTE ${header})
            list(APPEND headers ${header})
        else()
            list(APPEND headers ${CMAKE_CURRENT_SOURCE_DIR}/${header})
        endif()
    endforeach()

    add_custom_command(OUTPUT ${dstfile}
        COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/tarantool_header.h > ${tmpfile}
        COMMAND cat ${headers} | ${CMAKE_SOURCE_DIR}/extra/apigen >> ${tmpfile}
        COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/tarantool_footer.h >> ${tmpfile}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${tmpfile} ${dstfile}
        COMMAND ${CMAKE_COMMAND} -E remove ${tmpfile}
        DEPENDS ${srcfiles}
                ${CMAKE_CURRENT_SOURCE_DIR}/tarantool_header.h
                ${CMAKE_CURRENT_SOURCE_DIR}/tarantool_footer.h
        )

    add_custom_target(generate_module_api ALL DEPENDS ${srcfiles} ${dstfile})
    install(FILES ${dstfile} DESTINATION ${MODULE_INCLUDEDIR})
endfunction()

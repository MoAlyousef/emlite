cmake_minimum_required(VERSION 3.20)
project(emlite VERSION 0.1.0 LANGUAGES C CXX)

option(EMLITE_BUILD_EXAMPLES "Build examples" OFF)
option(EMLITE_BUILD_TESTS "Build tests" OFF)
option(EMLITE_USE_DLMALLOC "Use dlmalloc for freestanding environments" OFF)
option(EMLITE_USE_EMSCRIPTEN_JS_GLUE "Use emscripten's default js output" OFF)

add_library(emlite INTERFACE)

target_include_directories(emlite INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(emlite INTERFACE cxx_std_20)

set(DLMALLOC_LIB)
if (EMLITE_USE_DLMALLOC)
    add_library(dlmalloc src/dlmalloc.c)
    target_compile_definitions(dlmalloc PUBLIC 
        LACKS_UNISTD_H
        LACKS_FCNTL_H
        LACKS_SYS_PARAM_H
        LACKS_SYS_MMAN_H
        LACKS_STRINGS_H
        LACKS_STRING_H
        LACKS_SYS_TYPES_H
        LACKS_ERRNO_H
        LACKS_STDLIB_H
        LACKS_SCHED_H
        LACKS_TIME_H
        NO_MALLOC_STATS=1
        HAVE_MMAP=0
        HAVE_MORECORE=1
        MORECORE=sbrk
        MORECORE_CANNOT_TRIM=1
        MORECORE_CONTIGUOUS=1
        NO_MALLINFO=1
    )
    target_compile_definitions(emlite INTERFACE HAVE_DLMALLOC)
    target_link_libraries(emlite INTERFACE dlmalloc)
    set(DLMALLOC_LIB dlmalloc)
endif()

if(EMLITE_USE_EMSCRIPTEN_JS_GLUE)
    target_compile_definitions(emlite INTERFACE EMLITE_USE_EMSCRIPTEN_JS_GLUE)
endif()

add_library(emlite::emlite ALIAS emlite)

include(GNUInstallDirs)

install(TARGETS emlite ${DLMALLOC_LIB} EXPORT emliteTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT emliteTargets
    FILE emliteTargets.cmake
    NAMESPACE emlite::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emlite
)

if (EMLITE_BUILD_EXAMPLES OR EMLITE_BUILD_TESTS)
    set(DEFAULT_LINK_FLAGS)
    set(DEFAULT_LINK_FLAGS "-Wl,--no-entry,--allow-undefined,--export-dynamic,--export-if-defined=main,--export-table,--import-memory,--export-memory,--strip-all")
    if (EMSCRIPTEN)
        set(DEFAULT_LINK_FLAGS "${DEFAULT_LINK_FLAGS},--export-if-defined=_start -sERROR_ON_UNDEFINED_SYMBOLS=0 -sALLOW_MEMORY_GROWTH=1")
    endif()
    if (EMLITE_BUILD_EXAMPLES)
        add_executable(console examples/console.c)
        target_link_libraries(console PRIVATE emlite::emlite)
        set_target_properties(console PROPERTIES LINKER_LANGUAGE C SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})
        
        add_executable(eval examples/eval.cpp)
        target_link_libraries(eval PRIVATE emlite::emlite)
        set_target_properties(eval PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})

        add_executable(dom_simple examples/dom_simple.cpp)
        target_link_libraries(dom_simple PRIVATE emlite::emlite)
        set_target_properties(dom_simple PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})

        if (NOT USING_FREESTANDING)
            add_executable(audio examples/audio.cpp)
            target_link_libraries(audio PRIVATE emlite::emlite)
            set_target_properties(audio PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})

            add_executable(node_readfile examples/node_readfile.cpp)
            target_link_libraries(node_readfile PRIVATE emlite::emlite)
            set_target_properties(node_readfile PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})

            add_executable(bind examples/bind.cpp)
            target_link_libraries(bind PRIVATE emlite::emlite)
            set_target_properties(bind PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})
        endif()
    endif()

    if (EMLITE_BUILD_TESTS)
        if (NOT USING_FREESTANDING)
            if (NOT USING_WASI_LIBC)
                add_executable(dom_test1 tests/dom_test1.cpp)
                target_link_libraries(dom_test1 PRIVATE emlite::emlite)
                set_target_properties(dom_test1 PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})
            endif()
            add_executable(dom_test2 tests/dom_test2.c)
            target_link_libraries(dom_test2 PRIVATE emlite::emlite)
            set_target_properties(dom_test2 PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})
        endif()
        add_executable(dom_test1_nostdlib tests/dom_test1_nostdlib.cpp)
        target_link_libraries(dom_test1_nostdlib PRIVATE emlite::emlite)
        set_target_properties(dom_test1_nostdlib PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})

        add_executable(dom_test2_nostdlib tests/dom_test2_nostdlib.c)
        target_link_libraries(dom_test2_nostdlib PRIVATE emlite::emlite)
        set_target_properties(dom_test2_nostdlib PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS ${DEFAULT_LINK_FLAGS})
    endif()
endif()


find_package(Threads)

# Define a static library for Perfetto.
include_directories(sdk)
add_library(perfetto STATIC
    sdk/perfetto.h
    sdk/perfetto.cc
    )

if (WIN32)
  # The perfetto library contains many symbols, so it needs the big object
  # format.
  target_compile_options(perfetto PRIVATE "/bigobj")
  # Disable legacy features in windows.h.
  add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
  # On Windows we should link to WinSock2.
  target_link_libraries(example ws2_32)
endif (WIN32)

# Enable standards-compliant mode when using the Visual Studio compiler.
if (MSVC)
  target_compile_options(example PRIVATE "/permissive-")
endif (MSVC)

## example

add_executable(test_perfetto
    test/main.cpp
    )
target_link_libraries(test_perfetto
    perfetto ${CMAKE_THREAD_LIBS_INIT}
    )

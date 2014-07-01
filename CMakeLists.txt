cmake_minimum_required(VERSION 2.6.0)	
	
## Project information ##
project(ilcp_count CXX C)
ADD_DEFINITIONS(
      # -O0
      -g
      -O3
    # -DNDEBUG
      -std=c++0x # Or -std=c++0x
      -Wall
      -march=native
       )

include_directories(sdsl/include gflags/include
  sdsl/external/gtest-1.6.0/include)
link_directories(
  ${CMAKE_SOURCE_DIR}/sdsl/lib
  ${CMAKE_SOURCE_DIR}/gflags/lib
  ${CMAKE_SOURCE_DIR}/sdsl/external/gtest-1.6.0
)

add_subdirectory(sdsl)
add_subdirectory(wavelet)
add_subdirectory(gflags)

add_executable(suffix-array_test suffix-array_test.cpp suffix-array.cpp
  sais.cpp)
target_link_libraries(suffix-array_test
  gtest
  gtest_main
  pthread
  )

add_custom_command(TARGET suffix-array_test POST_BUILD
    COMMAND suffix-array_test || true)

add_executable(count_test count_test.cpp suffix-array.cpp sais.cpp
  wavelet/fast-bit-vector.cpp)
target_link_libraries(count_test
  gtest
  gtest_main
  pthread
  sdsl
  )

add_custom_command(TARGET count_test POST_BUILD
    COMMAND count_test || true)

add_executable(doc_counter doc_counter.cpp suffix-array.cpp sais.cpp
  wavelet/fast-bit-vector.cpp)

target_link_libraries(doc_counter
  sdsl
  gflags-static
  pthread
  )

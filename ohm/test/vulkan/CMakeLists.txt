
if(Build_Tests)
find_package(GTest)
if(GTest_FOUND)
  add_executable(test_ohm_vulkan test.cpp)
  target_link_libraries(test_ohm_vulkan GTest::GTest vulkan)
endif()
endif()

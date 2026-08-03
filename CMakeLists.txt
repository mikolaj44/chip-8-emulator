cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(TARGET_NAMES "raylib" "example" "all")

if(NOT TARGET_NAME IN_LIST TARGET_NAMES)
  message(FATAL_ERROR "TARGET_NAME needs to be one of: ${TARGET_NAMES}, pass -DTARGET_NAME=...")
endif()

project("chip-8-emulator")

function(INCLUDE_TARGET_FILES THIS_TARGET_NAME)
  file(
    GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS
    "include/components/core/*.hpp"
    "include/components/core/*.tpp"
    "include/components/custom/${THIS_TARGET_NAME}/*.hpp"
    "include/components/custom/${THIS_TARGET_NAME}/*.tpp"
    "include/utils/*.hpp"
    "include/utils/*.tpp"
    "src/${THIS_TARGET_NAME}/*.cpp"
    )

  add_executable(${EXECUTABLE_NAME} ${ALL_SOURCES})

  target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

  if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_STANDARD 26)
  endif()
endfunction()

if(TARGET_NAME STREQUAL "raylib" OR TARGET_NAME STREQUAL "all")
  set(EXECUTABLE_NAME "chip-8")

  set(FETCHCONTENT_QUIET FALSE)

  include(FetchContent)

  FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        dbc56a87da87d973a9c5baa4e7438a9d20121d28 # 6.0
    GIT_PROGRESS TRUE
    FIND_PACKAGE_ARGS
  )

  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        55f93686c01528224f448c19128836e7df245f72 # 3.12.0
    GIT_PROGRESS TRUE
    FIND_PACKAGE_ARGS
  )

  FetchContent_MakeAvailable(raylib nlohmann_json)

  INCLUDE_TARGET_FILES("raylib")

  target_link_libraries(${EXECUTABLE_NAME} PRIVATE raylib nlohmann_json::nlohmann_json)
endif()

if(TARGET_NAME STREQUAL "example" OR TARGET_NAME STREQUAL "all")
  set(EXECUTABLE_NAME "chip-8-example")

  INCLUDE_TARGET_FILES("example")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

cmake_minimum_required(VERSION 3.16)

project(
  shuguang
  LANGUAGES C
  VERSION 0.1.0
  DESCRIPTION "A shooting game engine"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(
  -Wall
  -Werror
  -Werror=maybe-uninitialized
  -Warray-bounds
  -Wstrict-prototypes
  -g
)

# set output directory
set(
  CMAKE_RUNTIME_OUTPUT_DIRECTORY
  "${CMAKE_BINARY_DIR}/../"
)

# find sdl3 library
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)  
find_package(SDL3_ttf REQUIRED)

# find lua
find_package(Lua)

if(Lua_FOUND AND NOT TARGET Lua::Lua)
  add_library(Lua::Lua INTERFACE IMPORTED)
  set_target_properties(
    Lua::Lua
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
  )
endif()

#### source code ####
set(SOURCES
  src/main.c
  src/graphic.c
)
#### source code ####

add_executable(${CMAKE_PROJECT_NAME}
  ${SOURCES}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
  SDL3::SDL3
  SDL3_image::SDL3_image
  SDL3_mixer::SDL3_mixer
  SDL3_ttf::SDL3_ttf

  Lua::Lua
)

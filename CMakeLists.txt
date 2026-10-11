cmake_minimum_required(VERSION 3.16)

# 项目信息
project(
  shuguang
  LANGUAGES C
  VERSION 0.1.0
  DESCRIPTION "A shooting game engine"
)

# 设置 C 语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译选项
add_compile_options(
  -Wall
  -Werror
  -Werror=maybe-uninitialized
  -Warray-bounds
  -Wstrict-prototypes
  -g
)

# 设置输入文件夹
set(
  CMAKE_RUNTIME_OUTPUT_DIRECTORY
  "${CMAKE_BINARY_DIR}/../"
)

# 查找 SDL3 及其扩展库
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)  
find_package(SDL3_ttf REQUIRED)

# 设置 glad 库的路径和头文件路径
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# 设置使用 libGL.so 
set(OpenGL_GL_PREFERENCE "LEGACY")

# 查找 OpenGL
find_package(OpenGL REQUIRED)

# 查找 Lua
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

#### 源码管理 ####
set(SOURCES
  src/main.c
  src/graphic.c
)
#### 源码管理 ####

# 添加生成可执行文件的目标
add_executable(${CMAKE_PROJECT_NAME}
  ${SOURCES}
)

# 链接库
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
  SDL3::SDL3
  SDL3_image::SDL3_image
  SDL3_mixer::SDL3_mixer
  SDL3_ttf::SDL3_ttf
  glad
  OpenGL::GL
  OpenGL::GLU
  Lua::Lua
)

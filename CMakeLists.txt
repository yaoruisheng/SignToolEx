cmake_minimum_required(VERSION 3.15)
project(SignToolExProject LANGUAGES CXX)

# Detours 源码路径
set(DETOURS_SRC
    detours/detours.cpp
	detours/disasm.cpp
    detours/modules.cpp
)
# 添加 Detours 头文件路径
include_directories(detours)
# 编译 detours 静态库
add_library(detours STATIC ${DETOURS_SRC})
#target_include_directories(detours PRIVATE detours)
#对于 detours 库，你已经通过 include_directories(detours) 添加了包含路径，再在 target_include_directories 中重复设置可能不必要。可以删除这一行。

# 添加可执行文件
add_executable(SignToolEx SignToolEx.cpp)

# 添加 DLL
add_library(SignToolExDll SHARED SignToolExDll.cpp)
target_link_libraries(SignToolExDll PRIVATE detours)

# 设置输出文件名（可选）
set_target_properties(SignToolExDll PROPERTIES OUTPUT_NAME "SignToolExHook")
set_target_properties(SignToolEx PROPERTIES OUTPUT_NAME "SignToolEx")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(SignToolEx PRIVATE _M_X64 DETOURS_X64)
    target_compile_definitions(SignToolExDll PRIVATE _M_X64 DETOURS_X64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_compile_definitions(SignToolEx PRIVATE _M_IX86 DETOURS_X86)
    target_compile_definitions(SignToolExDll PRIVATE _M_IX86 DETOURS_X86)
else()
    message(FATAL_ERROR "Unknown architecture")
endif()
cmake_minimum_required(VERSION 3.22)

# 设置项目名称
set(CMAKE_PROJECT_NAME freertos_balance_car)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# 设置 size 工具
set(SIZE arm-none-eabi-size)

# 定义构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# 导出编译命令（便于 clangd 等工具索引）
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# 彩色诊断
set(CMAKE_COLOR_DIAGNOSTICS ON)

# 包含工具链文件（必须在 project() 前设置编译器）
# include("cmake/gcc-arm-none-eabi.cmake")
include("cmake/gcc-arm-none-eabi.cmake")

# 核心项目设置
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# 启用 C / C++ / ASM 支持
enable_language(C CXX ASM)

# 去掉警告屏蔽（调试时可删除 -w）
add_compile_options(-w)

# 创建可执行文件
add_executable(${CMAKE_PROJECT_NAME})

# 链接 STM32CubeMX 生成的库
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    m
)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -lc_nano)

# 添加 STM32CubeMX 生成的源码
add_subdirectory(cmake/stm32cubemx)

# ---------------- 自定义函数：批量添加源码目录 -----------------
function(USER_ADD_DIR dir)
    file(GLOB USER_SOURCE
        ${dir}/*.h
        ${dir}/*.c
        ${dir}/*.hpp
        ${dir}/*.cpp
        ${dir}/*.cc
    )
    target_sources(${CMAKE_PROJECT_NAME} PUBLIC ${USER_SOURCE})
    target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${dir})
endfunction()

# ---------------- 添加用户代码目录 -----------------
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/App)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/App/control)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/App/mode)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/App/utils)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/App/task)

USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Bsp)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Bsp/gpio)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Bsp/delay)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Bsp/iic)

USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/key)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/led)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/beep)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/encoder)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/motor)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/oled)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/mpu6050)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/hc06)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/hcsr04)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/algorithm)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/algorithm/filter)
USER_ADD_DIR(${CMAKE_SOURCE_DIR}/Modules/algorithm/pid)

# ---------------- 链接选项（newlib-nano + printf浮点支持） -----------------
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC
    -lc_nano
    -u _printf_float
)

# ---------------- 编译完成后生成 hex / bin -----------------
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMENT "Building hex & bin files...\nExecutable size:"
    COMMAND ${CMAKE_OBJCOPY} -Oihex  $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${SIZE} $<TARGET_FILE:${PROJECT_NAME}>
)

cmake_minimum_required(VERSION 3.10)

# Compile in release mode by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Define the project.
project(elfloader LANGUAGES C CXX ASM)
add_compile_options(-Wno-unused-function)

# Add ABI files into include path.
include_directories(
	src
)

# Create output file and add sources.
add_library(elfloader
	src/relocation/reloc_riscv.cpp
	src/mpu/mpu_riscv32_pmp.cpp
	src/mpu.cpp
	src/relocation.cpp
	src/elfloader.cpp
)

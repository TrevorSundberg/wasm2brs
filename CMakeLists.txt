# Copyright 2020, Trevor Sundberg. See LICENSE.md
cmake_minimum_required(VERSION 3.13.4)
project(WASM2BRS VERSION 1.0.0)

add_subdirectory(third_party/wabt)
add_subdirectory(third_party/binaryen)

include_directories(${WABT_SOURCE_DIR} ${WABT_BINARY_DIR})

add_executable(wasm2brs src/wasm2brs.cc src/brs-writer.cc)

add_dependencies(wasm2brs wabt)
target_link_libraries(wasm2brs wabt)

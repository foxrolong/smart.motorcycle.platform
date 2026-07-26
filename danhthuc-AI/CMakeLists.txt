cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(PROJECT_VER "0.1.0")

# Khai báo partition table tùy chỉnh (có partition 'model' cho WakeNet)
set(PARTITION_TABLE_OFFSET "0x8000")

project(foxro)

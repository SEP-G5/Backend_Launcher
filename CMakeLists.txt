cmake_minimum_required(VERSION 3.10)
project(bike_launcher C)
set(CMAKE_C_STANDARD 99)

if (MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
else ()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -ggdb")
endif ()

include_directories(SYSTEM
  thirdparty
  )

set(SRCS
  src/main.c
  thirdparty/argparse/argparse.c
  )

add_executable(${PROJECT_NAME} ${SRCS})

if (WIN32)
  target_link_libraries(${PROJECT_NAME})
else ()
  target_link_libraries(${PROJECT_NAME})
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC
  src
  )

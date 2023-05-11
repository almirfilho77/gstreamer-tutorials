cmake_minimum_required(VERSION 3.5)

# Macro definition to print variables (debugging purposes)
macro(print_all_variables)
message(STATUS "print_all_variables------------------------------------------{")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

project(basics-5)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Uncomment the print_all_variables() function for debugging purposes
# print_all_variables()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(BIN_DIR        "${PROJECT_SOURCE_DIR}/bin")
set(INCLUDE_DIR    "${PROJECT_SOURCE_DIR}/inc")
set(RESOURCE_DIR   "${PROJECT_SOURCE_DIR}/res")
set(SOURCES_DIR    "${PROJECT_SOURCE_DIR}/src")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

include_directories(${INCLUDE_DIR})
include_directories(${GST_INCLUDE_DIRS})
include_directories(${GTK3_INCLUDE_DIRS})

link_directories(${GTK3_LIBRARY_DIRS})

add_compile_options(${GTK3_CFLAGS})

# message(STATUS "GST_VIDEO ${GST_VIDEO_LIBRARIES}")

file(GLOB SRCS  "${SOURCES_DIR}/*.cpp"
"${SOURCES_DIR}/*.c")

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} ${GST_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GST_VIDEO_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GST_APP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES})

#
# run: mkdir build && cd build && cmake .. && make 
#

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# project name
project(gst-opencv-simple-example LANGUAGES CXX)# CUDA)

# project version
if(NOT VERSION_MAJOR)
    set(VERSION_MAJOR 1)
endif()
if(NOT VERSION_MINOR)
    set(VERSION_MINOR 0)
endif()
if(NOT VERSION_PATCH)
    set(VERSION_PATCH 0)
endif()

message(STATUS "Build version is ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
add_definitions(-DVERSION_MAJOR=${VERSION_MAJOR} -DVERSION_MINOR=${VERSION_MINOR} -DVERSION_PATCH=${VERSION_PATCH})

# detect operating system
message(STATUS "We are on a ${CMAKE_SYSTEM_NAME} system")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DSYSTEM_LINUX)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_definitions(-DSYSTEM_DARWIN)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
    add_definitions(-DSYSTEM_AIX)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-DSYSTEM_WINDOWS)
endif()

# detect host processor
message(STATUS "The host processor is ${CMAKE_HOST_SYSTEM_PROCESSOR}")

# set c++ compiler flags for GNU
set(CMAKE_CXX_STANDARD 17) 
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)    
    set(CMAKE_CXX_FLAGS             "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -Wconversion -Wfatal-errors") # pedantic mode
    set(CMAKE_CXX_FLAGS             "${CMAKE_CXX_FLAGS} -Wunused-parameter")
    set(CMAKE_CXX_FLAGS             "${CMAKE_CXX_FLAGS} -rdynamic ")  # add symbols to stacktrace
    set(CMAKE_CXX_FLAGS             "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fopenmp")
    set(CMAKE_CXX_FLAGS_RELEASE     "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG       "-O0 -g3")
endif()

# pkg-config
find_package(PkgConfig REQUIRED)

# found dependencies libs
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.48)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)

# executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Pipeline.cpp
    # src/test3.cpp
)

# includes
set(INCLUDES
    # /home/user/sources/dep/dldt/inference-engine/temp/opencv_4.1.0_ubuntu16/include
    ${GST_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})

# libs
set(LIBS
    # /home/user/sources/dep/dldt/inference-engine/temp/opencv_4.1.0_ubuntu16/lib/libopencv_imgcodecs.so
    # /home/user/sources/dep/dldt/inference-engine/temp/opencv_4.1.0_ubuntu16/lib/libopencv_imgproc.so
    # /home/user/sources/dep/dldt/inference-engine/temp/opencv_4.1.0_ubuntu16/lib/libopencv_core.so

    libopencv_imgcodecs.so
    libopencv_imgproc.so
    libopencv_core.so
    ${GST_LIBRARIES}
    ${GLIB_LIBRARIES}
    z
    pthread
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

# make install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_DESTDIR}/bin)
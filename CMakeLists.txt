cmake_minimum_required(VERSION 3.20.0)

project(WebcamStreamer LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)

set(STREAMER_INCLUDE_DIRECTORY ${CMAKE_SOURCE_DIR}/include)
set(STREAMER_LIB_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Include directories
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GSTREAMER_VIDEO_INCLUDE_DIRS})
include_directories(${GSTREAMER_APP_INCLUDE_DIRS})

include_directories(${STREAMER_INCLUDE_DIRECTORY})

# link directories
link_directories(${GSTREAMER_LIBRARY_DIRS})
link_directories(${GSTREAMER_BASE_LIBRARY_DIRS})
link_directories(${GSTREAMER_VIDEO_LIBRARY_DIRS})
link_directories(${GSTREAMER_APP_LIBRARY_DIRS})

link_directories(${STREAMER_LIB_DIRECTORY})

set(SOURCES main.c ${STREAMER_LIB_DIRECTORY}/webcam_stream.c)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(main ${SOURCES})

# Link the GStreamer libraries to your executable
target_link_libraries(main ${GSTREAMER_LIBRARIES})
target_link_libraries(main ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(main ${GSTREAMER_VIDEO_LIBRARIES})
target_link_libraries(main ${GSTREAMER_APP_LIBRARIES})


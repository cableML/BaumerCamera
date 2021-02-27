cmake_minimum_required(VERSION 3.14)
project(baumer_camera)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

if ("${LIBRARY_INSTALL_DESTINATION}" STREQUAL "")
   set(LIBRARY_INSTALL_DESTINATION ${CMAKE_SOURCE_DIR}/host)
endif()

if("${CUSTOM_OPENCV_BUILD_PATH}" STREQUAL "")
   find_package(OpenCV REQUIRED)
else()
   find_package(OpenCV REQUIRED PATHS ${CUSTOM_OPENCV_BUILD_PATH} NO_DEFAULT_PATH)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
set(bgapi_DIR "C:/Program Files/Baumer GAPI SDK (64 Bit)/Components/Dev/C++/Lib/cmake/bgapi" CACHE STRING "Path to the cmake subfolder of bgapi-sdk installation directory")
else()
set(bgapi_DIR "/opt/baumer-gapi-sdk/lib/cmake/bgapi" CACHE STRING "Path to the cmake subfolder of bgapi-sdk installation directory")
endif()

find_package(bgapi 2.11 REQUIRED COMPONENTS genicam ext_sc)

include_directories(
        .
        include
        ${OpenCV_INCLUDE_DIRS})

set(${PROJECT_NAME}_PUBLIC_INCLUDES include/baumer_camera/BaumerCamera.hpp)

add_library(${PROJECT_NAME}
        ${${PROJECT_NAME}_PUBLIC_INCLUDES}
        src/BaumerCamera.cpp
        src/TimeMeasuring.hpp)

target_link_libraries(${PROJECT_NAME}
        bgapi::bgapi2_genicam
        ${OpenCV_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${${PROJECT_NAME}_PUBLIC_INCLUDES}")

install(TARGETS ${PROJECT_NAME}
   ARCHIVE DESTINATION ${LIBRARY_INSTALL_DESTINATION}/lib
   LIBRARY DESTINATION ${LIBRARY_INSTALL_DESTINATION}/lib
   RUNTIME DESTINATION ${LIBRARY_INSTALL_DESTINATION}/bin
   PUBLIC_HEADER DESTINATION ${LIBRARY_INSTALL_DESTINATION}/include/${PROJECT_NAME})

add_subdirectory(test)
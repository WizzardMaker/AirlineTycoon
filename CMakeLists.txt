cmake_minimum_required (VERSION 3.10)

project (AirlineTycoon)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT WIN32)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
endif()

set(SDL2_PATH "/usr/lib/")
find_package(SDL2 REQUIRED)

set(SDL2_MIXER_PATH "/usr/lib/")
find_package(SDL2-mixer REQUIRED)

set(SDL2_IMAGE_PATH "/usr/lib/")
find_package(SDL2-image REQUIRED)

set(SDL2_TTF_PATH "/usr/lib/")
find_package(SDL2-ttf REQUIRED)

# Include external libs.
add_subdirectory (libs)

# Include sub-projects.
add_subdirectory (src)

set_target_properties(AT PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(TeakLibW PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(SSE PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(SBLib PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(libsmacker PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(enet PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(smk2avi PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(driver PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(raknet PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")


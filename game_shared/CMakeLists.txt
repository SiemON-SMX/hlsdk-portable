cmake_minimum_required(VERSION 3.9)
project (VCS_INFO)

set(GIT_VERSION "unknown")
add_definitions(-DXASH_BUILD_COMMIT="unknown")

if(GIT_BRANCH)
	add_definitions(-DXASH_BUILD_BRANCH="${GIT_BRANCH}")
endif()


include_directories(.)
add_library(vcs_info STATIC vcs_info.c)
set_property(TARGET vcs_info PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

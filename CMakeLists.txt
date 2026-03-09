cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${_name} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${_name} PRIVATE OBS::libobs)

add_compile_options(/WX-)
include(FetchContent)
set(USE_TLS OFF CACHE BOOL "Disable TLS" FORCE)
set(USE_ZLIB OFF CACHE BOOL "Disable ZLIB" FORCE)
FetchContent_Declare(
  ixwebsocket
  GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
  GIT_TAG        v11.4.3
)
FetchContent_MakeAvailable(ixwebsocket)
target_link_libraries(${_name} PRIVATE ixwebsocket)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${_name} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${_name} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${_name}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${_name}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

target_sources(${_name} PRIVATE src/plugin-main.cpp)

set_target_properties_plugin(${_name} PROPERTIES OUTPUT_NAME ${_name})

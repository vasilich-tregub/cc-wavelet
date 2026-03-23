# CMakeList.txt : CMake project for cc-wavelet, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("cc-wavelet")

# Add source to this project's executable.
#add_executable (cc-wavelet "cc-wavelet.c" "cc-wavelet.h" "image-readwrite.h" "dwt.c" "dwt.h" "httpserverreceivepostrequest.1.0.cpp" "httpserverreceivepostrequest.1.0 (2).cpp")
if (WIN32)
add_executable (webui-cc-wavelet "webui-cc-wavelet.c" "cc-wavelet.c")
elseif (LINUX)
add_executable (webui-cc-wavelet "webui-cc-wavelet-ux.c" "cc-wavelet.c")
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET webui-cc-wavelet PROPERTY C_STANDARD 11)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/favicon128.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/index.html DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cc-wavelet.js DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/js-wavelet/NGC2207.jpg DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/js-wavelet/hubble-ngc4388-potw2550a.jpg DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/js-wavelet/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/js-wavelet/)

# TODO: Add tests and install targets if needed.

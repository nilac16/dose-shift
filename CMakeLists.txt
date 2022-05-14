cmake_minimum_required(VERSION 3.12)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

find_package(DCMTK COMPONENTS dcmrt REQUIRED)

if (MSVC)
    set(WIN_NATIVE WIN32)
    set(USER_CXX_FLAGS "/O2 /W3 -D_CRT_SECURE_NO_DEPRECATE /arch:AVX2")
    set(USER_C_FLAGS "/O2 /W3 -D_CRT_SECURE_NO_DEPRECATE /arch:AVX2 /std:c17")
else ()
    if (FULL_DEBUG)
        set(USER_CXX_FLAGS "-g3 -O0 -Wall -Wextra -fsanitize=address -fsanitize=undefined")
        set(USER_C_FLAGS "-g3 -O0 -Wall -Wextra -fsanitize=address -fsanitize=undefined")
    else ()
        set(USER_CXX_FLAGS "-O2 -Wall -Wextra")
        set(USER_C_FLAGS "-O2 -Wall -Wextra")
    endif ()
    set(USER_CXX_FLAGS "${USER_CXX_FLAGS} -mfma -mavx2")
    set(USER_C_FLAGS "${USER_C_FLAGS} -mfma -mavx2")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USER_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USER_C_FLAGS}")

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/ctrls)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/proton)

add_executable(${PROJECT_NAME} ${WIN_NATIVE}
    ${MAIN_SOURCES} ${CTRL_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE ${wxWidgets_LIBRARIES}
    PUBLIC proton)

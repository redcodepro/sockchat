cmake_minimum_required(VERSION 3.15)

set(BUILD_SHARED_LIBS OFF)

# enet
set(ENET_STATIC ON)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/enet)

# md5
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/md5)

# sqlite
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/sqlite3)

# sockchat
project(sockchat C CXX)

add_executable(sockchat
    main.cpp
    sc_funcs.cpp
    sqlite.cpp
    chat-handler.cpp
    cmd-handler.cpp
    _commands.cpp
    udp-cryptor.cpp
    udp-packet.cpp
    udp-server.cpp
    udp-user.cpp
)

set_target_properties(sockchat
    PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if (MSVC)
    target_compile_options(sockchat
        PRIVATE
            /utf-8
            /GS-
            /sdl-
    )
    target_compile_definitions(sockchat
        PRIVATE
            _CRT_NONSTDC_NO_DEPRECATE
            _CRT_SECURE_NO_WARNINGS
    )
endif()

target_link_libraries(sockchat
    PRIVATE
        enet_static
        md5
        sqlite3
)

target_include_directories(sockchat
    PRIVATE
        thirdparty/enet/include
        thirdparty/md5
        thirdparty/sqlite3
)

# CMake file for compiling the sqlite3 static library under Windows (for ease of use)
#
# Copyright (c) 2012-2023 Sebastien Rombauts (sebastien.rombauts@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)

project(sqlite3 C CXX)

add_library(sqlite3 sqlite3.c sqlite3.h)

if (WIN32)
    if (BUILD_SHARED_LIBS)
            add_definitions("-DSQLITE_API=__declspec(dllexport)")
    endif()
endif()

if (SQLITE_ENABLE_COLUMN_METADATA)
    # Enable the use of SQLite column metadata method
    # Require that the sqlite3 library is also compiled with this flag:
    target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_COLUMN_METADATA)
endif (SQLITE_ENABLE_COLUMN_METADATA)

if (SQLITE_ENABLE_RTREE)
    # Enable RTree extension when building sqlite3
    # See more here: https://sqlite.org/rtree.html
    target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_RTREE)
    message(STATUS "Compile sqlite3 with SQLITE_ENABLE_RTREE")
endif (SQLITE_ENABLE_RTREE)

if (SQLITE_ENABLE_DBSTAT_VTAB)
    # Enable DBSTAT extension when building sqlite3
    # See more here: https://www.sqlite.org/dbstat.html
    target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_DBSTAT_VTAB)
    message(STATUS "Compile sqlite3 with SQLITE_ENABLE_DBSTAT_VTAB")
endif (SQLITE_ENABLE_DBSTAT_VTAB)

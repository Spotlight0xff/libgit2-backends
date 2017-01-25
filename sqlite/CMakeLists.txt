PROJECT(LIBGIT2-sqlite C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(../CMake/FindLibgit2.cmake)
INCLUDE(../CMake/FindSQLite3.cmake)

# Build options
OPTION (BUILD_SHARED_LIBS "Build Shared Library (OFF for Static)" ON)
OPTION (BUILD_TESTS "Build Tests" ON)

# Build Release by default
IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF ()

# Compile and link LIBGIT2
INCLUDE_DIRECTORIES(${LIBGIT2_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS})
ADD_LIBRARY(git2-sqlite sqlite_odb_backend.c sqlite_refdb_backend.c vector.c pool.c)

# we need to prevent the linker from exporting git_pool_* and git_vector_*
SET_SOURCE_FILES_PROPERTIES(vector.c pool.c PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
TARGET_LINK_LIBRARIES(git2-sqlite ${LIBGIT2_LIBRARIES} ${SQLITE3_LIBRARIES})

ADD_EXECUTABLE(test_refdb ./tests/test_refdb_sqlite.c)
TARGET_LINK_LIBRARIES(test_refdb ${LIBGIT2_LIBRARIES} ${SQLITE3_LIBRARIES} git2-sqlite)

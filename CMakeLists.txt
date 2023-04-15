CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(tadis)
SET(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")


# SET(BOOST_ROOT /home/pink/boost/boost_1_81_0)
#SET(Boost_INCLUDE_DIRS /home/pink/boost/boost_1_81_0)
#LINK_DIRECTORIES("/home/pink/boost/boost_1_81_0/stage/lib")
#SET(Boost_LIBRARIES /home/pink/boost/boost_1_81_0/stage/lib)
#INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
## Find Boost
#FIND_PACKAGE(Boost 1.75.0)
#IF (Boost_FOUND)
#    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
#    MESSAGE(STATUS "Boost_INCLUDES_DIR = ${Boost_INCLUDE_DIRS}.")
#    MESSAGE(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
#    MESSAGE(STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
#ENDIF()

#SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/build/bin)

# For clangd ccls lsp
SET(DCMAKE_EXPORT_COMPILE_COMMANDS True)
SET(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -g -Wall -DCMAKE_EXPORT_COMPILE_COMMANDS=1")
IF(DEBUG)
    MESSAGE("DEBUG has been set as TRUE")
    ADD_DEFINITIONS(-DENABLE_DEBUG)
ENDIF()
## For logger
#ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK=1)

SET(CMAKE_CXX_FLAGS ${CMAKE_COMMON_FLAGS})
SET(CMAKE_C_FLAGS ${CMAKE_COMMON_FLAGS})
MESSAGE("CMAKE_CXX_FLAGS is " ${CMAKE_CXX_FLAGS})
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src/" SYSTEM)

add_executable(server src/bin/server.cc)

add_executable(stmt_test unit_test/stmt_test.cc)
add_executable(tuple_test unit_test/tuple_test.cc)
add_executable(buffer_pool_remove_test unit_test/table_heap_test.cc)
add_executable(json_test unit_test/json_test.cc)
add_executable(comparsion_expression_test unit_test/comparsion_expression_test.cc)
add_executable(seq_exec_test unit_test/seq_exec_test.cc)
add_executable(filter_expression_test unit_test/filter_expression_test.cc)
add_executable(projection_executer_test unit_test/projection_executer_test.cc)
add_executable(bind_test unit_test/bind_test.cc)
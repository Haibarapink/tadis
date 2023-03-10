CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(tadis)
SET(CMAKE_CXX_STANDARD 20)

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

SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/build/bin)

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

ADD_EXECUTABLE(tadis ${PROJECT_SOURCE_DIR}/src/main.cc)
#TARGET_LINK_LIBRARIES(tadis PRIVATE
#    pthread
#    boost_log
#    boost_thread
#    boost_log_setup
#)

# Tests
#ADD_EXECUTABLE(lex_test ${PROJECT_SOURCE_DIR}/unit_tests/lex_test.cc)
#ADD_EXECUTABLE(log_test ${PROJECT_SOURCE_DIR}/unit_tests/log_test.cc)
#ADD_EXECUTABLE(parse_test ${PROJECT_SOURCE_DIR}/unit_tests/parse_test.cc)
#ADD_EXECUTABLE(mem_store_test ${PROJECT_SOURCE_DIR}/unit_tests/memory_storage_test.cc)
#ADD_EXECUTABLE(tuple_test ${PROJECT_SOURCE_DIR}/unit_tests/tuple_test.cc)
#ADD_EXECUTABLE(json_test ${PROJECT_SOURCE_DIR}/unit_tests/json_test.cc)
#ADD_EXECUTABLE(utility_test ${PROJECT_SOURCE_DIR}/unit_tests/utility_test.cc)
#ADD_EXECUTABLE(rex_test ${PROJECT_SOURCE_DIR}/unit_tests/rex_test.cc)
#ADD_EXECUTABLE(lru_test ${PROJECT_SOURCE_DIR}/unit_tests/lru_test.cc)
#ADD_EXECUTABLE(disk_test ${PROJECT_SOURCE_DIR}/unit_tests/disk_manager_test.cc)
#ADD_EXECUTABLE(bitmap_test ${PROJECT_SOURCE_DIR}/unit_tests/bitmap_test.cc)
#ADD_EXECUTABLE(bfp_test ${PROJECT_SOURCE_DIR}/unit_tests/bfp_test.cc)
#ADD_EXECUTABLE(record_test ${PROJECT_SOURCE_DIR}/unit_tests/record_test.cc)
#ADD_EXECUTABLE(table_test ${PROJECT_SOURCE_DIR}/unit_tests/table_test.cc)
## tests config
#TARGET_LINK_LIBRARIES(log_test PRIVATE
#pthread
#boost_log
#boost_thread
#boost_log_setup
#)
#
#TARGET_LINK_LIBRARIES(parse_test PRIVATE
#    pthread
#    boost_log
#    boost_thread
#    boost_log_setup
#)
#
#TARGET_LINK_LIBRARIES(tuple_test PRIVATE
#    pthread
#    boost_log
#    boost_thread
#    boost_log_setup
#)
#
#TARGET_LINK_LIBRARIES(json_test PRIVATE
#    pthread
#    boost_log
#    boost_thread
#    boost_log_setup
#)
#
#
#TARGET_LINK_LIBRARIES(disk_test PRIVATE
#    pthread
#    boost_log
#    boost_thread
#    boost_log_setup
#)
#
#TARGET_LINK_LIBRARIES(bfp_test PRIVATE
#    pthread
#    boost_log
#    boost_thread
#    boost_log_setup
#)
#
#TARGET_LINK_LIBRARIES(record_test PRIVATE
#    pthread
#    boost_log
#    boost_thread
#    boost_log_setup
#)
#
#TARGET_LINK_LIBRARIES(table_test PRIVATE
#    pthread
#    boost_log
#    boost_thread
#    boost_log_setup
#)
#
#
## unit tests
#ENABLE_TESTING()
#ADD_TEST(
#    NAME boost_test
#    COMMAND $<TARGET_FILE:lex_test> $<TARGET_FILE:log_test> $<TARGET_FILE:parse_test>
#)


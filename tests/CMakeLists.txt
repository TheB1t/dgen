project(basic_tests)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests)
SET(TESTS_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/tests/resources)

SET(TESTS declarations functions arithmetic logical loops arrays switch struct)
FILE(GLOB_RECURSE TEST_SRC_FILES "*.c")

FOREACH(fname IN LISTS TEST_SRC_FILES)
    get_filename_component(FILE_NAME_NO_EXT ${fname} NAME_WE)

    SET(xNAME ${PROJECT_NAME}_${FILE_NAME_NO_EXT})
    ADD_EXECUTABLE(${xNAME} ${SRC_FILES} ${fname})

    FOREACH(test IN LISTS TESTS)
        IF(ENABLE_VALLGRIND)
            ADD_TEST(NAME ${xNAME}_${test} COMMAND valgrind --leak-check=full --track-origins=yes ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${xNAME} "${TESTS_RESOURCE_DIR}/test_${test}.dgen")
        ELSE()
            ADD_TEST(NAME ${xNAME}_${test} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${xNAME} "${TESTS_RESOURCE_DIR}/test_${test}.dgen")
        ENDIF()
    ENDFOREACH()
ENDFOREACH()
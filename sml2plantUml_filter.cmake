set(FILTER_SUFFIX "_state_machine_sml\\.hpp$")

# The header file path is passed as the last positional argument.
# Doxygen calls: <filter> <file>, so the file is always the last argument.
math(EXPR _LAST_ARG_INDEX "${CMAKE_ARGC} - 1")
set(HEADER "${CMAKE_ARGV${_LAST_ARG_INDEX}}")

if(NOT DEFINED HEADER OR HEADER STREQUAL "")
    message(FATAL_ERROR "No header file provided. Usage: cmake -P sml2plantUml_filter.cmake <header>")
endif()

get_filename_component(HEADER_ABS "${HEADER}" ABSOLUTE)
get_filename_component(FILE_NAME "${HEADER}" NAME)
string(
    REGEX REPLACE ${FILTER_SUFFIX}
    ""
    STATE_MACHINE_NAME
    "${FILE_NAME}"
)

set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(BUILD_DIR "${SOURCE_DIR}/build")

# Configure
# Forward environment variables for toolchain, extra CXX flags, and include directories if they are set
if(DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
    set(TOOLCHAIN_ARG "-DCMAKE_TOOLCHAIN_FILE=$ENV{CMAKE_TOOLCHAIN_FILE}")
endif()

if(DEFINED ENV{DOC_CXX_FLAGS})
    set(EXTRA_CXX_FLAGS "-DEXTRA_CXX_FLAGS=$ENV{DOC_CXX_FLAGS}")
endif()

if(DEFINED ENV{DOC_INCLUDE_DIRS})
    set(EXTRA_INCLUDE_DIRS "-DEXTRA_INCLUDE_DIRS=$ENV{DOC_INCLUDE_DIRS}")
endif()

execute_process(
    COMMAND
        cmake -S "${SOURCE_DIR}" -B "${BUILD_DIR}" ${TOOLCHAIN_ARG}
        ${EXTRA_CXX_FLAGS} ${EXTRA_INCLUDE_DIRS}
        -DHEADER_TO_CHECK="${HEADER_ABS}"
        -DSTATE_MACHINE_NAME="${STATE_MACHINE_NAME}"
    OUTPUT_QUIET
    RESULT_VARIABLE configure_result
)

if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "CMake configure failed")
endif()

# Build
execute_process(
    COMMAND cmake --build "${BUILD_DIR}"
    OUTPUT_QUIET
    RESULT_VARIABLE build_result
)

if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Build failed")
endif()

# Run
if(WIN32)
    set(APP_PATH "${BUILD_DIR}/StateMachine2Puml.exe")
else()
    set(APP_PATH "${BUILD_DIR}/StateMachine2Puml")
endif()

# Run the helper and capture its PlantUML output from stdout
execute_process(
    COMMAND "${APP_PATH}"
    OUTPUT_VARIABLE PUML_CONTENT
    RESULT_VARIABLE run_result
)

if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Execution failed")
endif()

# Build the documentation comment block.
string(CONCAT DOC_BLOCK
    "/**\n"
    "\\file\n"
    "${PUML_CONTENT}"
    "*/\n"
)

# Write original header with appended documentation block to stdout for Doxygen.
# CMake's message() writes to stderr, so we write to a temp file and use
# cmake -E cat which outputs to stdout.
file(READ "${HEADER_ABS}" CONTENT)
set(FILTER_OUTPUT_FILE "${BUILD_DIR}/_filter_output.tmp")
file(WRITE "${FILTER_OUTPUT_FILE}" "${CONTENT}\n${DOC_BLOCK}")
execute_process(COMMAND ${CMAKE_COMMAND} -E cat "${FILTER_OUTPUT_FILE}")

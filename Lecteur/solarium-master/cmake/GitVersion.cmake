# Check the optional git variable.
# If it's not set, we'll try to find it using the CMake packaging system.
if(NOT DEFINED GIT_EXECUTABLE)
    find_package(Git QUIET REQUIRED)
endif()

# Macro: RunGitCommand
# Description: short-hand macro for calling a git function. Outputs are the
#              "exit_code" and "output" variables.
macro(RunGitCommand)
    execute_process(COMMAND
            "${GIT_EXECUTABLE}" ${ARGV}
            WORKING_DIRECTORY "${GIT_DIR}"
            RESULT_VARIABLE exit_code
            OUTPUT_VARIABLE output
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT exit_code EQUAL 0)
        set(ENV{GIT_RETRIEVED_STATE} "false")
    endif()
endmacro()

RunGitCommand(log --pretty=format:'%h' -n 1)
set(GIT_REV ${output})

# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "Unknown")
    set(GIT_DIFF "")
    set(GIT_TAG "")
    set(GIT_BRANCH "")
else()
    #[[execute_process(
            COMMAND bash -c "git diff --quiet --exit-code || echo +"
            OUTPUT_VARIABLE GIT_DIFF)]]
    RunGitCommand(diff --quiet --exit-code || echo +)
    set(GIT_DIFF ${output})
    #[[execute_process(
            COMMAND git describe --exact-match --tags
            OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)]]
    RunGitCommand(describe --exact-match --tags)
    set(GIT_TAG ${output})
    #[[execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE GIT_BRANCH)]]
    RunGitCommand(rev-parse --abbrev-ref HEAD)
    set(GIT_BRANCH ${output})

    string(STRIP "${GIT_REV}" GIT_REV)
    string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
    string(STRIP "${GIT_DIFF}" GIT_DIFF)
    string(STRIP "${GIT_TAG}" GIT_TAG)
    string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
endif()

set(VERSION "#include \"GitVersion.h\"
const std::string GitVersion::Revision=\"${GIT_REV}${GIT_DIFF}\";
const std::string GitVersion::Tag=\"${GIT_TAG}\";
const std::string GitVersion::Branch=\"${GIT_BRANCH}\";
")

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/GitVersion.cpp)
    file(READ ${CMAKE_CURRENT_BINARY_DIR}/GitVersion.cpp VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/GitVersion.cpp "${VERSION}")
endif()

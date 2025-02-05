cmake_minimum_required(VERSION 3.22)
project(qe2ed
        VERSION 0.0.0.1
        DESCRIPTION "qe2ed"
        LANGUAGES C)

set(CMAKE_C_STANDARD 23)

include(FetchContent)
FetchContent_Declare(
        picoquic
        GIT_REPOSITORY https://github.com/hfstco/picoquic.git
        GIT_TAG        rtc
)
set(PICOQUIC_FETCH_PTLS ON)
FetchContent_MakeAvailable(picoquic)

include_directories(${picoquic_SOURCE_DIR}/picoquic)

add_library(qe2ed-core
        qe2ed/qe2ed_internal.c
        qe2ed/qe2ed_server.c
        qe2ed/qe2ed_client.c
        qe2ed/qe2ed_utils.c
)

add_executable(qe2ed main.c)

target_link_libraries(qe2ed picoquic-core picoquic-log qe2ed-core)

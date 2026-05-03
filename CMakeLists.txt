cmake_minimum_required(VERSION 3.16)
project(InfiniteGomoku C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

include(FetchContent)

set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/preview-3.1.3.zip
)

FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG preview-3.1.0
    GIT_SUBMODULES_RECURSE ON
)

FetchContent_MakeAvailable(SDL3 SDL3_ttf)

include_directories(Front Back)

add_executable(game 
    Front/main.c 
    Front/gui.c 
    Back/game_logic.c
)

target_link_libraries(game PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:game>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3_ttf::SDL3_ttf> $<TARGET_FILE_DIR:game>
)
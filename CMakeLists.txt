cmake_minimum_required(VERSION 4.3)
project(Tp_pokemon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Tp_pokemon main.cpp
        Inc/Pokemon.h
        src/Pokemon.cpp
        src/Pokemon_vector.cpp
        Inc/Pokemon_vector.h
        Inc/Pokedex.h
        src/Pokedex.cpp
)

find_package(SFML 2.6 COMPONENTS graphics window system audio  REQUIRED)

target_link_libraries(Tp_pokemon PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
)
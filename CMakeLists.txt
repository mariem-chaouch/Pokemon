cmake_minimum_required(VERSION 4.3)
project(Tp_pokemon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Tp_pokemon
	src/main.cpp
    Inc/Pokemon.h
    src/Pokemon.cpp
    Inc/Pokemon_vector.h
    src/Pokemon_vector.cpp
    Inc/Pokedex.h
    src/Pokedex.cpp
    Inc/Pokemon_Party.h
    src/Pokemon_Party.cpp
    Inc/Pokemon_Attack.h
    src/Pokemon_Attack.cpp
)

target_include_directories(Tp_pokemon PRIVATE Inc)

find_package(SFML 2.6 COMPONENTS graphics window system CONFIG REQUIRED)
target_link_libraries(Tp_pokemon PRIVATE sfml-graphics sfml-window sfml-system)

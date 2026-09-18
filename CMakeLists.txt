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
    Inc/Pokemon_party.h
    src/Pokemon_party.cpp
    Inc/Pokemon_attack.h
    src/Pokemon_attack.cpp
)

target_include_directories(Tp_pokemon PRIVATE Inc)

cmake_minimum_required(VERSION 4.3)
project(Tp_pokemon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Tp_pokemon
	src/main.cpp
	src/Etat.cpp
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

# Place les ressources a cote de l'executable, y compris dans les dossiers
# cmake-build-* utilises par CLion, quel que soit le repertoire de lancement.
add_custom_command(TARGET Tp_pokemon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Data"
        "$<TARGET_FILE_DIR:Tp_pokemon>/Data"
    COMMENT "Copie des ressources du jeu a cote de l'executable"
)

find_package(SFML 2.6 COMPONENTS graphics window system CONFIG REQUIRED)
target_link_libraries(Tp_pokemon PRIVATE sfml-graphics sfml-window sfml-system)

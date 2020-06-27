cmake_minimum_required(VERSION 2.8)
project(tetris)

aux_source_directory(src OMOK_SOURCE)

add_executable(tetris ${OMOK_SOURCE})
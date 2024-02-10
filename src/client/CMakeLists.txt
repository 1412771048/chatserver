aux_source_directory(. SRC)
add_executable(chatclient ${SRC})
target_link_libraries(chatclient pthread)
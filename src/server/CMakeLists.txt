aux_source_directory(. SRC)
add_executable(chatserver ${SRC})

target_link_libraries(chatserver muduo_net muduo_base pthread mysqlclient hiredis)
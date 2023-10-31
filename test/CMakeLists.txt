# 获取当前目录下的所有 .cc 文件
file(GLOB TEST_SOURCES "*.cc")

# 遍历每个测试源文件并创建对应的可执行文件
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})    
    target_link_libraries(${TEST_NAME} TJsonLib)
endforeach()
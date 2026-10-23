# Testes automatizados executados por `ctest --output-on-failure`.
# Usam somente Qt Core e nao fazem parte dos pacotes gerados.

if(BUILD_WITH_QT6)
    set(MD_TEST_QT_CORE Qt6::Core)
else()
    set(MD_TEST_QT_CORE Qt5::Core)
endif()

add_executable(test_url_allowlist test_url_allowlist.cpp)
target_include_directories(test_url_allowlist PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(test_url_allowlist ${MD_TEST_QT_CORE})
add_test(NAME url_allowlist COMMAND test_url_allowlist)

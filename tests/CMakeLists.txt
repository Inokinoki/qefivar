find_package(QT NAMES Qt6 Qt5 COMPONENTS Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

set(test_libraries QEFI Qt${QT_VERSION_MAJOR}::Test)

add_executable(test_parse_boot_order test_parse_boot_order.cc)
add_executable(test_parse_boot_name test_parse_boot_name.cc)

add_test("Parse Boot Order Test" test_parse_boot_order)
add_test("Parse Boot Name Test" test_parse_boot_name)

target_link_libraries(test_parse_boot_order ${test_libraries})
target_link_libraries(test_parse_boot_name ${test_libraries})

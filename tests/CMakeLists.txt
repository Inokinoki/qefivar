find_package(QT NAMES Qt6 Qt5 COMPONENTS Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

set(test_libraries QEFI Qt${QT_VERSION_MAJOR}::Test)

add_executable(test_parse_boot_order test_parse_boot_order.cc)
add_executable(test_parse_boot_name test_parse_boot_name.cc)
add_executable(test_parse_boot_guid test_parse_boot_guid.cc)
add_executable(test_parse_boot_path test_parse_boot_path.cc)

add_test("Parse Boot Order Test" test_parse_boot_order)
add_test("Parse Boot Name Test" test_parse_boot_name)
add_test("Parse Boot GUID Test" test_parse_boot_guid)
add_test("Parse Boot Path Test" test_parse_boot_path)

target_link_libraries(test_parse_boot_order ${test_libraries})
target_link_libraries(test_parse_boot_name ${test_libraries})
target_link_libraries(test_parse_boot_guid ${test_libraries})
target_link_libraries(test_parse_boot_path ${test_libraries})

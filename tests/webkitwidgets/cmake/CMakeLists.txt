
cmake_minimum_required(VERSION 2.8)

project(qmake_cmake_files)

enable_testing()

find_package(Qt6 COMPONENTS Core REQUIRED)

include("${_Qt6CTestMacros}")

set(Qt6_MODULE_TEST_DEPENDS Widgets)

test_module_includes(
  WebKit QWebElement
  WebKitWidgets QWebView
)

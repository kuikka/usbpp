cmake_minimum_required (VERSION 2.8.11)
project(libusbpp)
find_package(PkgConfig)

set(USBPP_CXX_FLAGS "-std=c++11" "-W" "-Wall" "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

add_library(usbpp
            bytebuffer.h
            events.cpp
            events.h
            hexdump.cpp
            hexdump.h
            usb/libusbpp.cpp
            usb/libusbpp.h
            usb/usb_configuration.cpp
            usb/usb_configuration.h
            usb/usb_context.cpp
            usb/usb_context.h
            usb/usb_device.cpp
            usb/usb_device.h
            usb/usb_device_handle.cpp
            usb/usb_device_handle.h
            usb/usb_endpoint.h
            usb/usb_interface.h
            )
target_include_directories(usbpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(usbpp PUBLIC ${USBPP_CXX_FLAGS} ${LIBUSB_CFLAGS})
target_link_libraries(usbpp LINK_PRIVATE ${LIBUSB_LIBRARIES})

link_directories(${LIBUSB_LIBRARY_DIRS})

add_executable(usbpptest
               usbpp.cpp
               bt/hci.cpp
               bt/hci.h)
target_compile_options(usbpptest PUBLIC ${USBPP_CXX_FLAGS})
target_link_libraries(usbpptest LINK_PUBLIC usbpp)


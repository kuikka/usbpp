#pragma once
#include <vector>
#include <stdint.h>
#include <libusb.h>
#include "usb_interface.h"

namespace usbpp
{

    class usb_configuration
    {
    public:
        usb_configuration( struct libusb_config_descriptor * );
        int nr_interfaces() const { return m_bNumInterfaces; }
        int value() const { return m_bConfigurationValue; }
        int attributes() const { return m_bmAttributes; }
        const std::vector<usb_interface>& interfaces() const { return m_interfaces; };

    private:
        struct libusb_config_descriptor m_desc;
        uint8_t m_bNumInterfaces;
        uint8_t m_bConfigurationValue;
        uint8_t m_bmAttributes;
        std::vector<usb_interface>      m_interfaces;
    };

};
// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

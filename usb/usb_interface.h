#pragma once

#include <vector>

#include "usb_endpoint.h"

namespace usbpp
{
    class usb_interface
    {
    public:
        usb_interface( const struct libusb_interface_descriptor *intf );
        const std::vector<usb_endpoint>& endpoints() const { return m_endpoints; };
        int number() const { return m_bInterfaceNumber; };
        int intf_class() const { return m_bInterfaceClass; };
        int subclass() const { return m_bInterfaceSubClass; };
        int protocol() const { return m_bInterfaceProtocol; };
        int alt() const { return m_bAlternateSetting; };

    private:
        uint8_t                   m_bInterfaceNumber;
        uint8_t                   m_bAlternateSetting;
        uint8_t                   m_bInterfaceClass;
        uint8_t                   m_bInterfaceSubClass;
        uint8_t                   m_bInterfaceProtocol;
        std::vector<usb_endpoint> m_endpoints;
    };

};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

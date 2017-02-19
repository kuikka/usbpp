#pragma once

#include <stdint.h>
#include <libusb.h>

namespace usbpp
{

    class usb_endpoint
    {
    public:
        usb_endpoint() {};
        usb_endpoint( const struct libusb_endpoint_descriptor *ep );
        int ep() const { return m_ep; };
        int transfer_type() const { return m_bmAttributes & LIBUSB_TRANSFER_TYPE_MASK; };
        bool is_in() const { return m_ep & LIBUSB_ENDPOINT_IN; }
        bool is_out() const { return !is_in(); };

    private:
        uint8_t         m_ep = 0;
        uint8_t         m_bmAttributes = 0;
        uint16_t        m_wMaxPacketSize = 0;
        uint8_t         m_bInterval = 0;
    };

};
// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

#include <iostream>
#include <libusb.h>

#include <poll.h>
#include <cstdio>
#include <cstring>

#include "libusbpp.h"

namespace usbpp
{
    /////////////////////////////////////////////////////////////////////
    ///////// ENDPOINT
    /////////////////////////////////////////////////////////////////////
    usb_endpoint::usb_endpoint( const struct libusb_endpoint_descriptor *ep )
        :       m_ep( ep->bEndpointAddress )
                ,       m_bmAttributes( ep->bmAttributes )
                ,       m_wMaxPacketSize( ep->wMaxPacketSize )
                ,       m_bInterval( ep->bInterval )
    {
    }

    /////////////////////////////////////////////////////////////////////
    ///////// INTERFACE
    /////////////////////////////////////////////////////////////////////
    usb_interface::usb_interface( const struct libusb_interface_descriptor *intf )
        :       m_bInterfaceNumber( intf->bInterfaceNumber )
                ,       m_bAlternateSetting( intf->bAlternateSetting )
                ,       m_bInterfaceClass( intf->bInterfaceClass )
                ,       m_bInterfaceSubClass( intf->bInterfaceSubClass )
                ,       m_bInterfaceProtocol( intf->bInterfaceProtocol )
    {
        for ( int i = 0; i < intf->bNumEndpoints; i++ )
        {
            m_endpoints.push_back( usb_endpoint( &intf->endpoint[i] ) );
        }
    }

};

std::ostream& operator<<(std::ostream& os, const usbpp::usb_device& device)
{
    auto f = os.setf(std::ios_base::hex);
    os << "VID: 0x" << std::hex << device.vid() << " PID: 0x" << std::hex << device.pid();
    os.setf( f );
    return os;
}

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

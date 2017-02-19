#include "usb_configuration.h"

namespace usbpp
{

    usb_configuration::usb_configuration( struct libusb_config_descriptor *desc )
        :       m_bNumInterfaces( desc->bNumInterfaces )
        ,       m_bConfigurationValue( desc->bConfigurationValue )
        ,       m_bmAttributes( desc->bConfigurationValue )
    {
        for ( int i = 0; i < desc->bNumInterfaces; i++ )
        {
            for ( int j = 0; j < desc->interface[i].num_altsetting; j++ )
            {
                m_interfaces.push_back( usb_interface( &desc->interface[i].altsetting[j] ) );
            }
        }
    }


};
// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

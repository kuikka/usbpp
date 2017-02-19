#include <iostream>
#include <functional>

#include <cstdio>
#include <cstring>

#include "usb_device.h"

namespace usbpp
{

    /////////////////////////////////////////////////////////////////////
    ///////// DEVICE
    /////////////////////////////////////////////////////////////////////
    usb_device::usb_device(struct libusb_device *dev)
    {
        m_dev = libusb_ref_device(dev);
        libusb_get_device_descriptor(m_dev, &m_desc);

        std::cout << "m_desc.bNumConfigurations " << (int)m_desc.bNumConfigurations << "\n";
        for ( int i = 0; i < m_desc.bNumConfigurations; i++ )
        {
            struct libusb_config_descriptor *config;
            int err = libusb_get_config_descriptor(m_dev,
                i,
                &config);
            if ( err )
            {
                std::cout << "libusb_get_config_descriptor ret=" << err << "\n";
                continue;
            }
            std::cout << "foo\n";
            m_configurations.push_back( usb_configuration( config ) );
            libusb_free_config_descriptor( config );
        }
    }

    usb_device::~usb_device()
    {
        libusb_unref_device(m_dev);
    }

    usb_device::usb_device(const usb_device &other)
        : m_configurations( other.m_configurations )
    {
        m_dev = libusb_ref_device(other.m_dev);
        libusb_get_device_descriptor(m_dev, &m_desc);
    }

};
// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

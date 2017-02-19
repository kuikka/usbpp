#pragma once

#include <libusb.h>
#include "usb_device_handle.h"
#include "usb_configuration.h"

namespace usbpp
{

    ///////////////////////////////////////////////////////////////////
    //// usbpp::usb_device
    ///////////////////////////////////////////////////////////////////
    class usb_device
    {
    public:
        usb_device(struct libusb_device *dev);
        ~usb_device();
        usb_device(const usb_device &other);

        uint16_t vid() const { return m_desc.idVendor; }
        uint16_t pid() const { return m_desc.idProduct; }
        int dev_class() const { return m_desc.bDeviceClass; }
        int subclass() const { return m_desc.bDeviceSubClass; }
        int protocol() const { return m_desc.bDeviceProtocol; }
        const std::vector<usb_configuration>& configurations() const { return m_configurations; };

        std::unique_ptr<usb_device_handle> open()
        {
            libusb_device_handle *h;
            int ret = libusb_open(m_dev, &h);
            printf("Libusb ret=%d\n", ret);
            return std::unique_ptr<usb_device_handle>( new  usb_device_handle( h ) );
        }

    private:
        struct libusb_device            *m_dev;
        struct libusb_device_descriptor m_desc;
        std::vector<usb_configuration>  m_configurations;
    };

};

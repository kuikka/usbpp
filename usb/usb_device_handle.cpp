#include <iostream>
#include <libusb.h>

#include <poll.h>
#include <cstdio>
#include <cstring>

#include "libusbpp.h"
#include "hexdump.h"

namespace usbpp
{

    /////////////////////////////////////////////////////////////////////
    ///////// HANDLE
    /////////////////////////////////////////////////////////////////////
    static void handle_libusb_transfer_cb_fn( struct libusb_transfer *transfer )
    {
        usb_device_handle *h = static_cast<usb_device_handle*>( transfer->user_data );
        h->on_libusb_transfer_cb( transfer );
    }

    bool usb_device_handle::transfer(const usbpp::usb_endpoint &ep,
        size_t length,
        transfer_cb_fn cb)
    {
        if ( ep.is_out() )
            return false;

        return transfer(ep, nullptr, length, cb);
    }

    bool usb_device_handle::transfer(const usbpp::usb_endpoint &ep,
        uint8_t *buffer,
        size_t length,
        // TODO: Copy data in write? Or claim buffer?
        transfer_cb_fn cb)
    {
        std::cout << "libusbpp::usb_device_handle::transfer(ep=0x" << std::hex << ep.ep() << ") buffer = " << (const void*)buffer << std::dec << " length=" << length << "\n";

        if ( ep.is_out() && !buffer )
            return false;

        xfer t;
        // allocate buffer if none provided, else use the provided buffre
        if ( buffer == nullptr )
        {
            t.buf = std::unique_ptr<uint8_t[]>( new uint8_t[length] );
            if ( ! t.buf )
                return false;

            buffer = t.buf.get();
        }

//        if ( ep.is_out() )
//            ::memcpy(buffer, buffer, length);

        t.xfer = std::unique_ptr<struct libusb_transfer, xfer::deleter>(
            libusb_alloc_transfer( 0 ) );
        if ( !t.xfer )
            return false;

        ::libusb_fill_interrupt_transfer(
            t.xfer.get(),
            m_handle.get(),
            ep.ep(),
            buffer,
            length,
            handle_libusb_transfer_cb_fn,
            this,
            0);

        std::cout << "type=" << (int) t.xfer.get()->type << "\n";

        int err = ::libusb_submit_transfer( t.xfer.get() );
        if ( err )
        {
            std::cout << "libusb_submit_transfer failed: " << err << "\n";
            return false;
        }

        t.cb = cb;
        m_transfers.push_back( std::move( t ) );
        return true;
    }

    bool usb_device_handle::claim_interface(int bInterfaceNumber)
    {
        int err = libusb_detach_kernel_driver(m_handle.get(), bInterfaceNumber);
        if (err) {
            std::cout << "libusb_detach_kernel_driver failed: " << err << "\n";
//            return false;
        }

        err = libusb_claim_interface(m_handle.get(), bInterfaceNumber);
        if (err) {
            std::cout << "libusb_claim_interface failed: " << err << "\n";
  //          return false;
        }
        return err == 0;
    }

    bool usb_device_handle::control_transfer(uint8_t bmRequestType,
        uint8_t bRequest,
        uint16_t wValue,
        uint16_t wIndex,
        uint16_t wLength,
        uint8_t *buffer,
        transfer_cb_fn cb)
    {
        xfer t;

        t.buf = std::unique_ptr<uint8_t[]>( new uint8_t[sizeof(struct libusb_control_setup) + wLength] );
        if ( ! t.buf )
            return false;

        t.xfer = std::unique_ptr<struct libusb_transfer, xfer::deleter>( libusb_alloc_transfer( 0 ) );
        if ( !t.xfer )
        {
            return false;
        }

        ::libusb_fill_control_setup(t.buf.get(), bmRequestType, bRequest,
            wValue, wIndex, wLength);
        // Copy data after
        if ( buffer )
        {
            ::memcpy(t.buf.get() + sizeof(struct libusb_control_setup),
                buffer,
                wLength);
        }

        libusb_fill_control_transfer(t.xfer.get(), m_handle.get(), t.buf.get(),
            handle_libusb_transfer_cb_fn, this , 1000);

        hexdump(t.buf.get(), wLength + sizeof(struct libusb_control_setup));

        int err = libusb_submit_transfer( t.xfer.get() );
        if ( err )
        {
            return false;
        }

        t.cb = cb;
        m_transfers.push_back( std::move( t ) );
        return true;
    }

    void usb_device_handle::on_libusb_transfer_cb( struct libusb_transfer *completed )
    {
        for ( auto it = m_transfers.begin();
            it != m_transfers.end();
            ++it )
        {
            if ( it->xfer.get() == completed )
            {
                xfer t = std::move( *it );
                m_transfers.erase( it );
                usbpp::usb_transfer_status status = usb_transfer_status::OK;
                if ( completed->status != LIBUSB_TRANSFER_COMPLETED )
                    status = usb_transfer_status::FAIL;
                t.cb( status, completed->buffer, completed->actual_length );
                return;
            }
        }
    }

    std::vector<usb_endpoint> usb_device_handle::endpoints()
    {
        std::vector<usb_endpoint> v;

        return v;
    }

};
// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

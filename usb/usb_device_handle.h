#pragma once

#include <vector>
#include <memory>

#include "usb_endpoint.h"

namespace usbpp
{
    enum class usb_transfer_status
    {
        OK,
        FAIL
    };

    ///////////////////////////////////////////////////////////////////
    //// usbpp::usb_device_handle
    //// this class encapsulates a libusb_device_handle
    ///////////////////////////////////////////////////////////////////
    class usb_device_handle
    {
    public:
        using transfer_cb_fn = std::function<void(
            usb_transfer_status transfer_status,
            const uint8_t *transferred_data,
            size_t actual_size)>;

        usb_device_handle(	libusb_device_handle *h ) : m_handle(h, ::libusb_close) {}

        bool control_transfer(uint8_t bmRequestType,
            uint8_t bRequest,
            uint16_t wValue,
            uint16_t wIndex,
            uint16_t wLength,
            uint8_t *buffer,
            transfer_cb_fn);

        /* Only for IN transfer, data will be returned
         * in a static buffer and it must be copied by caller */
        bool transfer(const usbpp::usb_endpoint &ep,
            size_t length,
            transfer_cb_fn);

        /* IN or OUT transfer.
         * If buffer is nullptr, buffer management will be taken care of.
         * If a buffer is passed it must be available until the callback */
        bool transfer(const usbpp::usb_endpoint &ep,
            const uint8_t *buffer,
            size_t length,
            transfer_cb_fn);

        std::vector<usb_endpoint> endpoints();
        bool claim_interface(int bInterfaceNumber);

    public:
        void on_libusb_transfer_cb(struct libusb_transfer*);

    private:
        struct xfer
        {
            struct deleter {
                void operator()(struct libusb_transfer* t) {
                     ::libusb_free_transfer( t );
                };
            };
            transfer_cb_fn cb;
            std::unique_ptr<struct libusb_transfer, deleter> xfer;
            std::unique_ptr<uint8_t[]> buf;
        };
        std::shared_ptr<libusb_device_handle>	m_handle;
        std::vector<xfer>                     m_transfers;
    };

};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

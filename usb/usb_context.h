#pragma once

#include <vector>

#include "events.h"
#include "libusbpp.h"

namespace usbpp
{
    class usb_device;

    ///////////////////////////////////////////////////////////////////
    //// usb::usb_context
    ///////////////////////////////////////////////////////////////////
    class usb_context : public events::event_receiver
    {
    public:
        usb_context();
        bool init( events::event_base *event_base, int vendor_id = -1, int product_id = -1 );
        std::vector<usb_device> devices();
        void set_hotplug_handler(std::function<void(usb_device, bool)>);

    public:
        void on_fd_added( int fd, short events );
        void on_fd_removed( int fd );
        void on_hotplug( libusb_device *device, libusb_hotplug_event event );

    public: // inherited from events::event_receiver
        virtual void event_callback( int, short ) override;

    private:
        using hotplug_fn_t = std::function<void(usb_device&, int)>; 

        libusb_context	               *m_ctx = nullptr;
        events::event_base             *m_base = nullptr;
        libusb_hotplug_callback_handle	m_hotplug_handle;
        std::list<hotplug_fn_t>         m_hotplug_handler;
    };

};
// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

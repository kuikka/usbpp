#include <iostream>
#include <libusb.h>

#include <poll.h>
#include <cstdio>
#include <cstring>

#include "libusbpp.h"
#include "usb_context.h"

namespace usbpp
{

    static void added_cb( int fd, short events, void *user_data )
    {
        printf("%s %d %x %p\n", __FUNCTION__, fd, events, user_data );
        usb_context *ctx = static_cast<usb_context*>( user_data );
        ctx->on_fd_added( fd, events );
    }

    static void remove_cb( int fd, void *user_data )
    {
        printf("%s %d %p\n", __FUNCTION__, fd, user_data );
        usb_context *ctx = static_cast<usb_context*>( user_data );
        ctx->on_fd_removed( fd );
    }

    static int hotplug_callback_fn(libusb_context* /* ctx */, libusb_device *device,
        libusb_hotplug_event event, void *user_data)
    {
        printf("hotplug_callback_fn() event=%x\n", event);
        usb_context *c = static_cast<usb_context*>( user_data );
        c->on_hotplug( device, event );

        return 0;
    }

    usb_context::usb_context()
    {
    }

    bool usb_context::init( events::event_base *event_base,int vendor_id, int product_id )
    {
        int ret = libusb_init( &m_ctx );
        if (ret)
        {
            printf("libusb_init failure: %d\n", ret);
            return false;
        }
        printf("libusb ctx %p\n", m_ctx);

        libusb_set_debug( m_ctx, LIBUSB_LOG_LEVEL_DEBUG );
        m_base = event_base;

        ::libusb_set_pollfd_notifiers (m_ctx,
            added_cb,
            remove_cb,
            this);

        const struct libusb_pollfd **fds = ::libusb_get_pollfds(m_ctx);
        for ( int i = 0;
            fds[i];
            i++)
        {
            printf("fd = %p\n", fds[i]);
            m_base->register_event(fds[i]->fd, fds[i]->events, this);
        }

        ::libusb_free_pollfds(fds);

        libusb_hotplug_register_callback(m_ctx,
            static_cast< libusb_hotplug_event >( LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT ),
            static_cast< libusb_hotplug_flag >( LIBUSB_HOTPLUG_ENUMERATE ),
            vendor_id == -1 ? LIBUSB_HOTPLUG_MATCH_ANY : vendor_id,
            product_id == -1 ? LIBUSB_HOTPLUG_MATCH_ANY : product_id,
            LIBUSB_HOTPLUG_MATCH_ANY,
            hotplug_callback_fn,
            this,
            &m_hotplug_handle);

        return true;
    }

    void usb_context::on_fd_added( int fd, short events )
    {
        printf("usbpp::usb_context::on_fd_added %d %x\n", fd, events);
        m_base->register_event(fd, events, this);
    }

    void usb_context::on_fd_removed( int fd )
    {
        printf("usbpp::usb_context::on_fd_removed %d\n", fd);
        m_base->unregister_event(fd, this);
    }

    void usb_context::on_hotplug( libusb_device *device, libusb_hotplug_event event )
    {
        printf("usbpp::usb_context::on_hotplug() event=%x\n", event);
        usbpp::usb_device dev(device);
        for ( auto& h : m_hotplug_handler )
            h( dev, event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED );
    }

    void usb_context::event_callback( int fd, short flags )
    {
        printf("usbpp::usb_context::event_callback %d %x\n", fd, flags);
        struct timeval tv = { 0, 0 };
        ::libusb_handle_events_timeout(m_ctx, &tv);
    }

    std::vector<usb_device> usb_context::devices()
    {
        std::vector<usb_device> devices;

        libusb_device **list;
        ssize_t cnt = libusb_get_device_list(m_ctx, &list);
        ssize_t i = 0;

        for (i = 0; i < cnt; i++)
        {
            libusb_device *dev = list[i];
            devices.push_back( usb_device(dev) );
        }
        ::libusb_free_device_list(list, 1);

        return devices;
    }

    void usb_context::set_hotplug_handler(std::function<void(usb_device, bool)> f)
    {
        m_hotplug_handler.push_back( f );
    }

}; // namespace usbpp

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

#include <iostream>
#include <libusb.h>

#include <poll.h>
#include <cstdio>
#include "libusbpp.h"

static void hexdump(const void *ptr, size_t len)
{
    uint8_t *p = (uint8_t*) ptr;
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x ", p[i]);
        if (i && i % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}

namespace events {

    void base::on_event( int fd, short events )
    {
        auto it = m_receivers.find( fd );
        if ( it != m_receivers.end() )
        {
            short flags = std::get<1>( it->second );
            receiver *r = std::get<0>( it->second );
            if (r && (events & flags))
                r->event_callback( fd, events );
        }
    }

    bool base::register_event( int fd, short flags, events::receiver *r )
    {
        printf("base::register_event %d %x %p\n", fd, flags, r);
        m_receivers[ fd ] = std::make_tuple( r, flags );
        return true;
    }

    void base::unregister_event( int fd, events::receiver *r )
    {
        auto i = m_receivers.find( fd );
        if ( i != m_receivers.end() )
        {
            auto p = (*i).second;
            if ( std::get<0>( p ) == r )
            {
                m_receivers.erase( i );
            }
        }
    }

    base::base()
    {
    }

    bool base::init()
    {
        return true;
    }

    void base::dispatch()
    {
        m_running = true;

        while(m_running)
        {
            size_t nfds = m_receivers.size();

            struct pollfd fds[nfds];

            size_t i = 0;
            for ( const auto& p : m_receivers )
            {
                fds[i].fd = p.first;
                fds[i].events = std::get<1>( p.second );
                fds[i].revents = 0;
            }

            int ret = ::poll( fds, nfds, 1000 );
            printf("Poll returned %d\n", ret);
            if ( ret > 0 )
            {
                for ( i = 0; i < nfds; ++i )
                {
                    if ( fds[i].revents )
                    {
                        on_event( fds[i].fd, fds[i].revents );
#if 0
                        auto it = m_receivers.find( fds[i].fd );
                        if ( it != m_receivers.end() )
                        {
                            auto r = std::get<0>( it->second );
                            short flags = std::get<1>( it->second );
                            int fd = fds[i].fd;
                            r->event_callback( fd, flags );
                        }
#endif
                    }
                }
            }
        }
    };

};


namespace usbpp
{

    static void added_cb( int fd, short events, void *user_data )
    {
        printf("%s %d %x %p\n", __FUNCTION__, fd, events, user_data );
        context *ctx = static_cast<context*>( user_data );
        ctx->on_fd_added( fd, events );
    }

    static void remove_cb( int fd, void *user_data )
    {
        printf("%s %d %p\n", __FUNCTION__, fd, user_data );
        context *ctx = static_cast<context*>( user_data );
        ctx->on_fd_removed( fd );
    }

    static int hotplug_callback_fn(libusb_context* /* ctx */, libusb_device *device,
            libusb_hotplug_event event, void *user_data)
    {
        printf("hotplug_callback_fn() event=%x\n", event);
        context *c = static_cast<context*>( user_data );
        c->on_hotplug( device, event );

        return 0;
    }

    context::context()
    {
    }

    bool context::init( events::base *event_base,int vendor_id, int product_id )
    {
        int ret = libusb_init( &m_ctx );
        if (ret)
        {
            printf("libusb_init failure: %d\n", ret);
            return false;
        }
        printf("libusb ctx %p\n", m_ctx);

//        libusb_set_debug( m_ctx, LIBUSB_LOG_LEVEL_DEBUG );
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

    void context::on_fd_added( int fd, short events )
    {
        printf("usbpp::context::on_fd_added %d %x\n", fd, events);
        m_base->register_event(fd, events, this);
    }

    void context::on_fd_removed( int fd )
    {
        printf("usbpp::context::on_fd_removed %d\n", fd);
        m_base->unregister_event(fd, this);
    }

    void context::on_hotplug( libusb_device *device, libusb_hotplug_event event )
    {
        printf("usbpp::context::on_hotplug() event=%x\n", event);
        usbpp::device dev(device);
        for ( auto& h : m_hotplug_handler )
            h( dev, event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED );
    }

    void context::event_callback( int fd, short flags )
    {
        printf("usbpp::context::event_callback %d %x\n", fd, flags);
        struct timeval tv = { 0, 0 };
        ::libusb_handle_events_timeout(m_ctx, &tv);
    }

    std::vector<device> context::devices()
    {
        std::vector<device> devices;

        libusb_device **list;
        ssize_t cnt = libusb_get_device_list(m_ctx, &list);
        ssize_t i = 0;

        for (i = 0; i < cnt; i++)
        {
            libusb_device *dev = list[i];
            devices.push_back( device(dev) );
        }
        ::libusb_free_device_list(list, 1);

        return devices;
    }

    void context::set_hotplug_handler(std::function<void(device, bool)> f)
    {
        m_hotplug_handler.push_back( f );
    }

    /////////////////////////////////////////////////////////////////////
    ///////// ENDPOINT
    /////////////////////////////////////////////////////////////////////
    endpoint::endpoint( const struct libusb_endpoint_descriptor *ep )
        :       m_ep( ep->bEndpointAddress )
        ,       m_bmAttributes( ep->bmAttributes )
        ,       m_wMaxPacketSize( ep->wMaxPacketSize )
        ,       m_bInterval( ep->bInterval )
    {
    }

    /////////////////////////////////////////////////////////////////////
    ///////// INTERFACE
    /////////////////////////////////////////////////////////////////////
    interface::interface( const struct libusb_interface_descriptor *intf )
        :       m_bInterfaceNumber( intf->bInterfaceNumber )
        ,       m_bAlternateSetting( intf->bAlternateSetting )
        ,       m_bInterfaceClass( intf->bInterfaceClass )
        ,       m_bInterfaceSubClass( intf->bInterfaceSubClass )
        ,       m_bInterfaceProtocol( intf->bInterfaceProtocol )
    {
        for ( int i = 0; i < intf->bNumEndpoints; i++ )
        {
            m_endpoints.push_back( endpoint( &intf->endpoint[i] ) );
        }
    }

    /////////////////////////////////////////////////////////////////////
    ///////// CONFIGURATION
    /////////////////////////////////////////////////////////////////////
    
    configuration::configuration( struct libusb_config_descriptor *desc )
        :       m_bNumInterfaces( desc->bNumInterfaces )
        ,       m_bConfigurationValue( desc->bConfigurationValue )
        ,       m_bmAttributes( desc->bConfigurationValue )
    {
        for ( int i = 0; i < desc->bNumInterfaces; i++ )
        {
            for ( int j = 0; j < desc->interface[i].num_altsetting; j++ )
            {
                m_interfaces.push_back( interface( &desc->interface[i].altsetting[j] ) );
            }
        }
    }

    /////////////////////////////////////////////////////////////////////
    ///////// DEVICE
    /////////////////////////////////////////////////////////////////////

    device::device(struct libusb_device *dev)
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
            m_configurations.push_back( configuration( config ) );
            libusb_free_config_descriptor( config );
        }
    }

    device::~device()
    {
        libusb_unref_device(m_dev);
    }

    device::device(const device &other)
        : m_configurations( other.m_configurations )
    {
        m_dev = libusb_ref_device(other.m_dev);
        libusb_get_device_descriptor(m_dev, &m_desc);
    }

    /////////////////////////////////////////////////////////////////////
    ///////// HANDLE
    /////////////////////////////////////////////////////////////////////
    static void handle_libusb_transfer_cb_fn( struct libusb_transfer *transfer )
    {
        handle *h = static_cast<handle*>( transfer->user_data );
        h->on_libusb_transfer_cb( transfer );
    }

    bool handle::transfer(const usbpp::endpoint &ep,
        uint8_t *buffer,
        size_t length,
        transfer_cb_fn cb)
    {
        xfer t;

        t.buf = std::unique_ptr<uint8_t[]>( new uint8_t[length] );
        if ( ! t.buf )
            return false;

        if ( ep.is_out() )
        {
            ::memcpy(t.buf.get(), buffer, length);
        }

        t.xfer = std::unique_ptr<struct libusb_transfer, xfer::deleter>(
             libusb_alloc_transfer( 0 ) );
        if ( !t.xfer )
        {
            return false;
        }

        ::libusb_fill_interrupt_transfer(
            t.xfer.get(),
            m_handle.get(),
            ep.ep(),
            t.buf.get(),
            length,
            handle_libusb_transfer_cb_fn,
            this,
            1000
        );

        int err = libusb_submit_transfer( t.xfer.get() );
        if ( err )
        {
            return false;
        }

        t.cb = cb;
        m_transfers.push_back( std::move( t ) );
        return true;
    }

    bool handle::control_transfer(uint8_t bmRequestType,
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

    void handle::on_libusb_transfer_cb( struct libusb_transfer *completed )
    {
        for ( auto it = m_transfers.begin();
                it != m_transfers.end();
                ++it )
        {
            if ( it->xfer.get() == completed )
            {
                xfer t = std::move( *it );
                m_transfers.erase( it );
                t.cb( completed->status, completed->buffer, completed->length );
                return;
            }
        }
    }

    std::vector<endpoint> handle::endpoints()
    {
        std::vector<endpoint> v;

        return v;
    }
};

std::ostream& operator<<(std::ostream& os, const usbpp::device& device)
{
    auto f = os.setf(std::ios_base::hex);
    os << "VID: 0x" << std::hex << device.vid() << " PID: 0x" << std::hex << device.pid();
    os.setf( f );
    return os;
}

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

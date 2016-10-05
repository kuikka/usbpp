#pragma once

#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <list>

#include <libusb.h>

struct libusb_context;

namespace events {

    class receiver
    {
    public:
        virtual void event_callback( int, short ) = 0;
    };

    class base
    {
    public:
        base();
        bool init();
        void on_event( int fd, short events );
        void dispatch();

    public:
        bool register_event( int fd, short flags, receiver *r );
        void unregister_event( int fd, receiver *r );

    protected:
        std::map< int, std::tuple< receiver*, short > >	m_receivers;
        bool	m_running = false;
    };

};

namespace usbpp
{

    class endpoint
    {
    public:
        endpoint() {};
        endpoint( const struct libusb_endpoint_descriptor *ep );
        int ep() const { return m_ep; };
        int transfer_type() const { return m_bmAttributes & LIBUSB_TRANSFER_TYPE_MASK; };
        bool is_in() const { return m_ep & LIBUSB_ENDPOINT_IN; }

    private:
        uint8_t         m_ep = 0;
        uint8_t         m_bmAttributes = 0;
        uint16_t        m_wMaxPacketSize = 0;
        uint8_t         m_bInterval = 0;
    };

    class handle
    {
    public:
        using transfer_cb_fn = std::function<void(const uint8_t*, size_t)>;

        handle(	libusb_device_handle *h ) : m_handle(h, ::libusb_close) {}

        bool control_transfer(uint8_t bmRequestType,
                uint8_t bRequest,
                uint16_t wValue,
                uint16_t wIndex,
                uint16_t wLength,
                uint8_t *buffer,
                transfer_cb_fn);

        std::vector<endpoint> endpoints();

    public:
        void on_libusb_transfer_cb(struct libusb_transfer*);

    private:
        struct transfer
        {
            transfer_cb_fn cb;
            struct libusb_transfer *t;
        };
        std::shared_ptr<libusb_device_handle>	m_handle;
        std::vector<transfer>                   m_transfers;
    };

    class interface
    {
    public:
        interface( const struct libusb_interface_descriptor *intf );
        const std::vector<endpoint>& endpoints() const { return m_endpoints; };
        int number() const { return m_bInterfaceNumber; };
        int intf_class() const { return m_bInterfaceClass; };
        int subclass() const { return m_bInterfaceSubClass; };
        int protocol() const { return m_bInterfaceProtocol; };
        int alt() const { return m_bAlternateSetting; };

    private:
        uint8_t                 m_bInterfaceNumber;
        uint8_t                 m_bAlternateSetting;
        uint8_t                 m_bInterfaceClass;
        uint8_t                 m_bInterfaceSubClass;
        uint8_t                 m_bInterfaceProtocol;
        std::vector<endpoint>   m_endpoints;
    };

    class configuration
    {
    public:
        configuration( struct libusb_config_descriptor * );
        int nr_interfaces() const { return m_bNumInterfaces; }
        int value() const { return m_bConfigurationValue; }
        int attributes() const { return m_bmAttributes; }
        const std::vector<interface>& interfaces() const { return m_interfaces; };

    private:
        struct libusb_config_descriptor m_desc;
        uint8_t m_bNumInterfaces;
        uint8_t m_bConfigurationValue;
        uint8_t m_bmAttributes;
        std::vector<interface>      m_interfaces;
    };

    class device
    {
    public:
        device(struct libusb_device *dev);
        ~device();
        device(const device &other);

        uint16_t vid() const { return m_desc.idVendor; }
        uint16_t pid() const { return m_desc.idProduct; }
        int dev_class() const { return m_desc.bDeviceClass; }
        int subclass() const { return m_desc.bDeviceSubClass; }
        int protocol() const { return m_desc.bDeviceProtocol; }
        const std::vector<configuration>& configurations() const { return m_configurations; };

        std::unique_ptr<handle> open()
        {
            libusb_device_handle *h;
            libusb_open(m_dev, &h);
            return std::unique_ptr<handle>( new  handle( h ) );
        }

    private:
        struct libusb_device		*m_dev;
        struct libusb_device_descriptor m_desc;
        std::vector<configuration>      m_configurations;
    };

    class context : public events::receiver
    {
    public:
        context();
        bool init( events::base *event_base, int vendor_id = -1, int product_id = -1 );
        std::vector<device> devices();
        void set_hotplug_handler(std::function<void(device, bool)>);

    public:
        void on_fd_added( int fd, short events );
        void on_fd_removed( int fd );
        void on_hotplug( libusb_device *device, libusb_hotplug_event event );

    public: // inherited from events::receiver
        virtual void event_callback( int, short );

    private:
        using hotplug_fn_t = std::function<void(device&, int)>; 
        libusb_context	*m_ctx = nullptr;
        events::base	*m_base = nullptr;
        libusb_hotplug_callback_handle	m_hotplug_handle;
        std::list<hotplug_fn_t> m_hotplug_handler;
    };

};
std::ostream& operator<<(std::ostream& os, const usbpp::device& device);

// vim: set shiftwidth=4 expandtab cinoptions=>1s,t0,g0:

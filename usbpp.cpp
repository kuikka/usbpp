#include <unistd.h>
#include <iostream>
#include "libusbpp.h"

using std::cout;
using std::unique_ptr;

void callback( usbpp::device device, bool arrived )
{
    std::cout << "USB device " << device << (arrived ? " added" : " removed") << std::endl;
    cout << "  Class " << std::hex << device.dev_class() << "\n";
    cout << "  Subclass " << std::hex << device.subclass() << "\n";
    cout << "  Protocol " << std::hex << device.protocol() << "\n";

    for ( auto config : device.configurations() )
    {
        cout << "    Configuration " << config.value() << "\n";
        for ( auto intf : config.interfaces() )
        {
            cout << "      interface " << intf.number() << "\n";
            cout << "        alt " << intf.alt() << "\n";
            cout << "        class " << intf.intf_class() << "\n";
            cout << "        subclass " << intf.subclass() << "\n";
            cout << "        protocol " << intf.protocol() << "\n";
            for ( auto ep : intf.endpoints() )
            {
                std::cout << "          ep: " << std::hex << (int)ep.ep() << " type: " << ep.transfer_type() << "\n";
            }
        }
    }

}

namespace hci
{
    class controller
    {
        virtual bool reset() = 0;
    };

    class usb_controller : public controller
    {
    public:
        usb_controller( usbpp::device& device )
            : m_dev( device )
        {
        }

        bool init()
        {
            // Find endpoints
            for ( auto config : m_dev.configurations() )
            {
                for ( auto intf : config.interfaces() )
                {
                    bool found_event_in = false;
                    bool found_data_in = false;
                    bool found_data_out = false;

                    for ( auto ep : intf.endpoints() )
                    {
                        if ( ep.is_in() && ep.transfer_type() == 3 ) {
                            m_event_in = ep;
                            found_event_in = true;
                        }
                        if ( ep.is_in() && ep.transfer_type() == 2 ) {
                            m_data_in = ep;
                            found_data_in = true;
                        }
                        if ( !ep.is_in() && ep.transfer_type() == 2 ) {
                            m_data_out = ep;
                            found_data_out = true;
                        }
                    }

                    if ( found_event_in && found_data_in && found_data_out )
                    {
                        cout << "Got it\n";
                        m_handle = m_dev.open();
                        return true;
                    }
                }
            }
            return false;
        }

        virtual bool reset()
        {
            return false;
        }

    private:
        usbpp::device                   m_dev;
        unique_ptr<usbpp::handle>       m_handle;
        usbpp::endpoint                 m_event_in;
        usbpp::endpoint                 m_data_out;
        usbpp::endpoint                 m_data_in;
    };

    class controller_factory
    {
    };

    class manager
    {
    public:
        static manager& get()
        {
            static manager m;
            return m;
        }

        bool add( std::shared_ptr<hci::controller> c )
        {
            m_controllers.push_back( c );
            return true;
        }

    private:
        std::vector< std::shared_ptr<hci::controller> > m_controllers;
    };

    class usb_controller_factory : public controller_factory
    {
    public:
        usb_controller_factory( usbpp::context &ctx )
            : controller_factory()
            ,  m_ctx(ctx)
            , m_manager( manager::get() )
        {
            m_ctx.set_hotplug_handler( [this]
                    (usbpp::device device, bool added)
                    {
                        this->hotplug_cb_fn( device, added );
                    } );
        }

        void hotplug_cb_fn( usbpp::device device, bool added )
        {
            std::cout << "FACTORY: USB device " << device << (added ? " added" : " removed") << std::endl;

            if ( added )
                probe( device );
        }

        void probe( usbpp::device& device )
        {
            if ( ( device.dev_class() == 0xE0
                        && device.subclass() == 0x01
                        && device.protocol() == 0x01 )
                    || ( device.vid() == 0x0b05
                        && device.pid() ==  0x17cb ) )
            {
                hci::usb_controller *c = new hci::usb_controller( device );
                if ( c->init() )
                    m_manager.add( std::shared_ptr<hci::controller>( c ) );
            }
        }

    private:
        usbpp::context  &m_ctx;
        hci::manager    &m_manager;
    };

}

int main(int /* argc */, char** /* *argv[] */)
{
    events::base b;
    usbpp::context c;

    b.init();

    hci::usb_controller_factory factory( c );

    c.set_hotplug_handler(&callback);
    c.init(&b, 0x0b05, 0x17cb );

#if 0
    auto devices = c.devices();
    for ( auto& device : devices )
    {
        printf("Device: %x %x\n", device.vid(), device.pid());
        for ( auto config : device.configurations() )
        {
            for ( auto intf : config.interfaces() )
            {
                for ( auto ep : intf.endpoints() )
                {
                    std::cout << "ep: " << std::hex << (int)ep.ep() << "\n";
                }
            }
        }
    }
#endif

    b.dispatch();
}

// vim: set shiftwidth=4 expandtab cinoptions=>1s,t0,g0:

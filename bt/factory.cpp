#include <iostream>

#include "factory.h"

using std::cout;

namespace hci
{

    usb_controller_factory::usb_controller_factory( usbpp::usb_context &ctx )
        : controller_factory()
          , m_ctx(ctx)
          , m_manager( manager::get() )
    {
        m_ctx.set_hotplug_handler(
            [this] (usbpp::usb_device device, bool added)
            {
                this->hotplug_cb_fn( device, added );
            });
    }


    void usb_controller_factory::hotplug_cb_fn( usbpp::usb_device device, bool added )
    {
        std::cout << "FACTORY: USB device " << device << (added ? " added" : " removed") << std::endl;
        if ( added )
            probe( device );
    }

    void usb_controller_factory::probe( usbpp::usb_device& device )
    {
        if ( ( device.dev_class() == 0xE0
                && device.subclass() == 0x01
                && device.protocol() == 0x01 )
            || ( device.vid() == 0x0b05
                && device.pid() ==  0x17cb ) )
        {
            std::shared_ptr<hci::controller> c( new hci::usb_controller( device ) );
            c->init(
                [c, this]( hci::controller::completed_status status ) {
                    cout << "Controller initialized success=" << status << "\n";
                    if (status == true) 
                        m_manager.add( c );
                });
        }
    }

};
// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

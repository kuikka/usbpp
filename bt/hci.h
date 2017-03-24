#pragma once

#include <vector>
#include <list>
#include "usb/libusbpp.h"
#include "bytebuffer.h"
#include "hci_event.h"
#include "hci_command.h"
#include "hci_controller.h"

using std::unique_ptr;
using std::shared_ptr;

namespace hci
{
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

        bool add( shared_ptr<hci::controller> c )
        {
            m_controllers.push_back( c );
            return true;
        }

    private:
        std::vector< shared_ptr<hci::controller> > m_controllers;
    };

    class usb_controller_factory : public controller_factory
    {
    public:
        usb_controller_factory( usbpp::usb_context &ctx );
        void hotplug_cb_fn( usbpp::usb_device device, bool added );
        void probe( usbpp::usb_device& device );

    private:
        usbpp::usb_context  &m_ctx;
        hci::manager        &m_manager;
    };

}

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

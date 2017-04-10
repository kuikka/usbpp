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
};
// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

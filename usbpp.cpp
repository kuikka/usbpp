#include <unistd.h>
#include <iostream>
#include "usb/libusbpp.h"
#include "bt/hci.h"

using std::cout;

void callback( usbpp::usb_device device, bool arrived )
{
    cout << "USB device " << device << (arrived ? " added" : " removed") << std::endl;
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

int main(int /* argc */, char** /* *argv[] */)
{
    events::event_base b;
    usbpp::usb_context c;

    b.init();

    hci::usb_controller_factory factory( c );

//    c.set_hotplug_handler(&callback);
    c.init(&b, 0x0a12, 0x0001 );

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

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

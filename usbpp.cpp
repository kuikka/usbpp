#include <unistd.h>
#include <iostream>
#include "usb/libusbpp.h"
#include "bt/hci.h"
#include "bt/factory.h"

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

static const uint64_t HCI_DEFAULT_EVENT_MASK = 0x2000000000000000L | 0x0000000000008000L | 0x0000000000000010L;
static const uint64_t HCI_DEFAULT_LE_EVENT_MASK = 0x1F;

class ble_address
{
public:
    ble_address();
    ble_address( const bytebuffer &bb, bool is_random = false ) : m_is_random(is_random)
    {
        for (int i = 0; i < 6; i++ )
            m_addr[i] = bb.get_byte();
    }

private:
    bool        m_is_random = false;
    uint8_t     m_addr[6] = { 0, 0, 0, 0, 0, 0 };
    friend std::ostream& operator<<(std::ostream &s, const ble_address &addr);
};

std::ostream& operator<<(std::ostream &s, const ble_address &addr)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x %c",
        addr.m_addr[5],
        addr.m_addr[4],
        addr.m_addr[3],
        addr.m_addr[2],
        addr.m_addr[1],
        addr.m_addr[0],
        addr.m_is_random ? 'r' : 'p');

    s << buf;
    return s;
}

class le_advertisement
{
public:
    le_advertisement( const ble_address &addr, int rssi, const bytebuffer &data )
        : m_address(addr)
          , m_rssi(rssi)
    {
        while (data.remaining() >= 3)
        {
            int len = data.get_byte();
            int type = data.get_byte();

            cout << "Adv from " << m_address << " type " << type << " length " << len << "\n";
            if (len > 1)
            {
                len--;
                while(len--)
                    data.get_byte();
            }
        }
    }

private:
    std::map< int, std::vector<uint8_t> >  m_data;
    ble_address m_address;
    int m_rssi;
};

class le_controller
{
public:
    le_controller( hci::controller &ctrl )
        : m_ctrl(ctrl)
    {
        m_ctrl.set_event_callback( std::bind(&le_controller::on_hci_event, this, std::placeholders::_1) );
    }

    void on_hci_event( const hci::event &ev )
    {
        cout << "le_controller::on_hci_event " << ev.code() << "\n";
        switch (ev.code())
        {
            case hci::event::LE_META_EVENT:
                {
                    int sub_event_code = ev.data().get_byte();
                    switch (sub_event_code)
                    {
                        case 2: // LE Advertising Report
                            {
                                int num_reports = ev.data().get_byte();

                                for ( int i = 0; i < num_reports; i++ )
                                {
                                    int event_type = ev.data().get_byte();
                                    int address_type = ev.data().get_byte();
                                    ble_address addr( ev.data(), address_type == 0 ? false : true );
                                    int length_data = ev.data().get_byte();
                                    bytebuffer data( ev.data(), length_data );
                                    int rssi = (int8_t) ev.data().get_byte();
                                    le_advertisement adv( addr, rssi, data );
                                }

                            }
                            break;
                    }
                }
                break;
        }
    }

protected:
    hci::controller     &m_ctrl;
};

std::list< shared_ptr<le_controller> > le_controllers;

void controller_added_cb( hci::controller &ctrl )
{
    std::cout << "New HCI controller added " << ctrl.name() << "\n";

    ctrl.reset( [&ctrl] ( bool success ) {
        std::cout << "Controller reset " << success << "\n";

        ctrl.submit_command( hci::command::set_event_mask( HCI_DEFAULT_EVENT_MASK ), [&ctrl] ( const hci::command& cmd ) {

            std::cout << "set_event_mask completed status=" << cmd.status() << "\n";
            ctrl.submit_command( hci::command::le_set_event_mask( HCI_DEFAULT_LE_EVENT_MASK ), [&ctrl] ( const hci::command& cmd ) {

                std::cout << "le_set_event_mask completed status=" << cmd.status() << "\n";
                ctrl.submit_command( hci::command::le_set_scan_parameters( true, 16, 16, false, false ), [&ctrl] ( const hci::command &cmd ) {

                    std::cout << "le_set_scan_parameters status=" << cmd.status() << "\n";
                    le_controllers.push_back( shared_ptr< le_controller >( new le_controller( ctrl ) ) );

                    ctrl.submit_command( hci::command::le_set_scan_enable( true, false ), [&ctrl] ( const hci::command &cmd ) {
                        std::cout << "le_set_enable status=" << cmd.status() << "\n";
                    });
                });
            });
        });
    });
}

int main(int /* argc */, char** /* *argv[] */)
{
    events::event_base b;
    usbpp::usb_context c;
    hci::manager &mgr = hci::manager::get();

    mgr.set_controller_added_cb( controller_added_cb );

    b.init();

    // Register usb controller factory first so it can get the hotplug events from usb context...
    hci::usb_controller_factory factory( c );

//    c.set_hotplug_handler(&callback);
    c.init( &b );
    //c.init(&b, 0x0a12, 0x0001 );

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

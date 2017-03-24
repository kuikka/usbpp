#include <iostream>
#include "hci.h"
#include "hexdump.h"

using std::cout;

namespace hci
{
    uint8_t OGF_LINK_CONTROL(0x00);
    uint8_t OGF_CONTROLLER(0x03);
    uint8_t OGF_LE(0x08);
    // Controller & baseband commands
    uint16_t OCF_SET_EVENT_MASK(0x0001);
    uint16_t OCF_RESET(0x0003);
    // LE commands
    uint16_t OCF_LE_SET_EVENT_MASK(0x0001);

    ///////////////////////////////////////////////////////////////////
    //// hci::command
    ///////////////////////////////////////////////////////////////////
    hci_command command::reset()
    {
        hci_command cmd( new command( OGF_CONTROLLER, OCF_RESET ) );
        return cmd;
    }

    hci_command command::le_set_event_mask( uint64_t le_event_mask )
    {
        hci_command cmd( new command( OGF_LE, OCF_LE_SET_EVENT_MASK ) );
        cmd->buffer().put( le_event_mask );
        return cmd;
    }

    ///////////////////////////////////////////////////////////////////
    //// hci::usb_controller
    ///////////////////////////////////////////////////////////////////
    usb_controller::usb_controller( usbpp::usb_device& device )
        : m_dev( device )
          , m_event_buffer( new uint8_t[ 512 ] )
    {
    }

    bool usb_controller::init( controller::completed_cb cb )
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
                    if ( ep.is_in() && ep.transfer_type() == LIBUSB_TRANSFER_TYPE_INTERRUPT ) {
                        m_event_in = ep;
                        found_event_in = true;
                    }
                    if ( ep.is_in() && ep.transfer_type() == LIBUSB_TRANSFER_TYPE_BULK ) {
                        m_data_in = ep;
                        found_data_in = true;
                    }
                    if ( !ep.is_in() && ep.transfer_type() == LIBUSB_TRANSFER_TYPE_BULK ) {
                        m_data_out = ep;
                        found_data_out = true;
                    }
                }

                if ( found_event_in && found_data_in && found_data_out )
                {
                    cout << "Got it\n";
                    m_handle = m_dev.open();
                    cout << "Opened\n";
                    m_handle->claim_interface(intf.number());
                    cout << "Claimed\n";
                    this->submit_event_transfer();
                    this->reset( cb );
                    return true;
                }
            }
        }
        return false;
    }

    bool usb_controller::submit_event_transfer()
    {
        bool success = m_handle->transfer(m_event_in,
            m_event_buffer.get(),
            64,
            std::bind(&usb_controller::on_event_transfer_completed, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3) );
        if ( !success )
            cout << "submit_event_transfer failed\n";
        return success;
    }

    void usb_controller::on_event_transfer_completed(usbpp::usb_transfer_status status,
        const uint8_t *buffer, size_t length)
    {
        cout << "Got USB Event status=" << (int) status << " buffer=" << buffer << " length=" << length << "\n";
        hci::event e( buffer, length );
        on_hci_event( e );
        submit_event_transfer();
    }


    usb_controller_factory::usb_controller_factory( usbpp::usb_context &ctx )
        : controller_factory()
          ,  m_ctx(ctx)
          , m_manager( manager::get() )
    {
        m_ctx.set_hotplug_handler(
            [this] (usbpp::usb_device device, bool added)
            {
                this->hotplug_cb_fn( device, added );
            });
    }

    bool usb_controller::send(hci::command *cmd)
    {
        cout << "Sending " << cmd->buffer().length() << " bytes\n";
        //        hexdump(cmd->buffer().ptr(), cmd->buffer().length());
        return m_handle->control_transfer(
            LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
            0, 0, 0,
            cmd->buffer().length(),
            cmd->buffer().ptr(),
            [](usbpp::usb_transfer_status status, const uint8_t *buf, size_t len) {
                std::cout << "Usb transfer done, status=" << (int) status << "\n";
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
                    cout << "Controller initialized\n";
                    if (status == true) 
                        m_manager.add( c );
                });
        }
    }

};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

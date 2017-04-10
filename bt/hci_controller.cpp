#include <iostream>
#include <cstring>

#include "hci.h"
#include "hci_command.h"
#include "hexdump.h"

using std::cout;

namespace hci
{
    ///////////////////////////////////////////////////////////////////
    //// hci::controller
    ///////////////////////////////////////////////////////////////////
    bool controller::reset( controller::operation_completed_cb cb )
    {
        submit_command( command::reset(),
            [=] (const hci::command &cmd)
            {
                if (cmd.status() == 0x00) {
                    cb( true );
                } else {
                    cb( false );
                }
            } );
        return true;
    }

    void controller::complete_command()
    {
        m_current_command->complete();
        m_current_command.reset();
        m_command_ongoing = false;

        send_next_command();
    }

    void controller::on_hci_event( const event &ev )
    {
        cout << "Got HCI event " << ev.code() << "\n";

        switch( ev.code() )
        {
            case hci::event::COMMAND_COMPLETED:
                {
                    int numHciCommandPackets = ev.data().get_byte();
                    int opcode = ev.data().get_word();
                    // parameters to follow
                    //
                    cout << "Command completed\n";
                    int status = ev.data().get_byte();

                    m_current_command->set_status( status );
                    
                    complete_command();
                }
                break;
            case hci::event::COMMAND_STATUS:
                {
                    int status = ev.data().get_byte();
                    int numHciCommandPackets = ev.data().get_byte();
                    int opcode = ev.data().get_word();

                    m_current_command->set_status( status );

                    complete_command();
                }
                break;

            default:
                if (m_event_callback)
                    m_event_callback(ev);
                break;
        };
    }

    void controller::set_event_callback(event_cb_t cb)
    {
        m_event_callback = cb;
    }

    void controller::submit_command( hci_command cmd, hci_command_completion_cb_t cb )
    {
        cmd->set_completion_cb( cb );
        submit_command( std::move( cmd ) );
    }

    void controller::submit_command(hci_command cmd)
    {
        m_command_queue.push_back( std::move( cmd ) );
        send_next_command();
    }

    void controller::send_next_command()
    {
        if ( !m_command_ongoing && !m_command_queue.empty() )
        {
            m_current_command = std::move( m_command_queue.front() );
            m_command_queue.pop_front();

            send( m_current_command.get() );
            m_command_ongoing = true;
        }
    }


    ///////////////////////////////////////////////////////////////////
    //// hci::usb_controller
    ///////////////////////////////////////////////////////////////////
    usb_controller::usb_controller( usbpp::usb_device& device )
        : m_dev( device )
          , m_event_buffer( new uint8_t[ 512 ] )
    {
        m_name = "usb controller";
    }

    bool usb_controller::init( controller::operation_completed_cb cb )
    {
        cout << "usb_controller::init()\n";
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
                    if (!m_handle->claim_interface(intf.number())) {
                        cout << "Claiming interface failed\n";
                        cb( false );
                        return false;
                    }

                    cout << "Claimed\n";
                    if (!this->submit_event_transfer()) {
                        cout << "Submit event tranfer failed\n";
                        cb( false );
                        return false;
                    }

                    cb( true );
                    return true;
                }
            }
        }
        return false;
    }

    bool usb_controller::submit_event_transfer()
    {
        cout << "usb_controller::submit_event_transfer()\n";

        ::memset( m_event_buffer.get(), 0, 512 );

        bool success = m_handle->transfer(m_event_in,
            m_event_buffer.get(),
            512,
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
        cout << "Got USB Event status=" << (int) status << " buffer=" << (void*)buffer << " length=" << length << "\n";
        hexdump(buffer, length);
        if (status == usbpp::usb_transfer_status::OK )
        {
            hci::event e( buffer, length );
            on_hci_event( e );
        }
        submit_event_transfer();
    }


    bool usb_controller::send( hci::command *cmd )
    {
        cout << "Sending " << cmd->buffer().length() << " bytes\n";
        hexdump(cmd->buffer().ptr(), cmd->buffer().length());

        return m_handle->control_transfer(
            LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
            0, 0, 0,
            cmd->buffer().length(),
            cmd->buffer().ptr(),
            [](usbpp::usb_transfer_status status, const uint8_t *buf, size_t len) {
                std::cout << "Usb transfer done, status=" << (int) status << "\n";
            });  
    }

};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

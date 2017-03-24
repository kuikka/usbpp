#include <iostream>
#include "hci.h"
#include "hci_command.h"
#include "hexdump.h"

using std::cout;

namespace hci
{
    ///////////////////////////////////////////////////////////////////
    //// hci::controller
    ///////////////////////////////////////////////////////////////////
    bool controller::reset( controller::completed_cb cb )
    {
        submit_command( command::reset(),
            [=] (const hci_command &cmd)
            {
                if (cmd->status() == 0x00) {
                    cb( true );
                } else {
                    cb( false );
                }
            } );
        return true;
    }

    void controller::on_hci_event( const event &ev )
    {
        cout << "Got HCI event " << ev.code() << "\n";
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
            auto &cmd = m_command_queue.front();
            send( cmd.get() );
            m_command_ongoing = true;
        }
    }
};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

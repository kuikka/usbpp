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


};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

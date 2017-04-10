#include "hci_command.h"

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
    uint16_t OCF_LE_READ_BUFFER_SIZE(0x0002);
    uint16_t OCF_LE_READ_LOCAL_SUPPORTED_FEATURES(0x0003);
    uint16_t OCF_LE_SET_SCAN_PARAMETERS(0x000B);
    uint16_t OCF_LE_SET_SCAN_ENABLE(0x000C);

    ///////////////////////////////////////////////////////////////////
    //// hci::command
    ///////////////////////////////////////////////////////////////////
    hci_command command::reset()
    {
        hci_command cmd( new command( OGF_CONTROLLER, OCF_RESET ) );
        return cmd;
    }

    hci_command command::set_event_mask(uint64_t event_mask)
    {
        hci_command cmd( new command( OGF_CONTROLLER, OCF_SET_EVENT_MASK ) );
        cmd->buffer().put( event_mask );
        cmd->set_length( 8 );
        return cmd;
    }

    hci_command command::le_set_event_mask( uint64_t le_event_mask )
    {
        hci_command cmd( new command( OGF_LE, OCF_LE_SET_EVENT_MASK ) );
        cmd->buffer().put( le_event_mask );
        cmd->set_length( 8 );
        return cmd;
    }

    hci_command command::le_set_scan_parameters( bool active, uint16_t interval,
        uint16_t window, bool own_addr_random, bool whitelist_only )
    {
        hci_command cmd( new command( OGF_LE, OCF_LE_SET_SCAN_PARAMETERS ) );
        cmd->buffer().put( static_cast< uint8_t >( active ? 1 : 0 ) );
        cmd->buffer().put( interval );
        cmd->buffer().put( window );
        cmd->buffer().put( static_cast< uint8_t >( own_addr_random ? 1 : 0 ) );
        cmd->buffer().put( static_cast< uint8_t >( whitelist_only ? 1 : 0 ) );
        cmd->set_length( 7 );

        return cmd;
    }

    hci_command command::le_set_scan_enable( bool enable, bool filter_duplicates )
    {
        hci_command cmd( new command( OGF_LE, OCF_LE_SET_SCAN_ENABLE ) );
        cmd->buffer().put( static_cast< uint8_t >( enable ? 1 : 0 ) );
        cmd->buffer().put( static_cast< uint8_t >( filter_duplicates ? 1 : 0 ) );
        cmd->set_length( 2 );

        return cmd;
    }

    void command::complete()
    {
        m_cb(*this);
    }
};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

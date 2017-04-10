#pragma once

#include <memory>
#include <functional>
#include "bytebuffer.h"

using std::unique_ptr;
using std::shared_ptr;

namespace hci
{
    class command;

    using hci_command = unique_ptr<command>;
    using hci_command_completion_cb_t = std::function<void(const hci::command&)>;

    class command
    {
    public:
        command( uint16_t ogf, uint16_t ocf ) : m_opcode(ogf << 10 | ocf)
        {
            m_cmd.put( m_opcode );
            m_cmd.put( (uint8_t) 0 ); // data length
        };

        int status() const { return m_status; }
        void set_status(int status) { m_status = status; }
        void set_completion_cb(hci_command_completion_cb_t cb) { m_cb = cb; }
        void complete();
        void set_length(int length) { m_cmd.ptr()[2] = length; };
        bytebuffer& buffer() { return m_cmd; };

    public:
        static hci_command reset();
        static hci_command set_event_mask( uint64_t le_event_mask );
        static hci_command le_set_event_mask( uint64_t le_event_mask );
        static hci_command le_set_scan_parameters( bool active, uint16_t interval, uint16_t window,
            bool own_addr_random = false, bool whitelist_only = false );
        static hci_command le_set_scan_enable( bool enable, bool filter_duplicates );

    protected:

    protected:
        hci_command_completion_cb_t   m_cb;
        bytebuffer                    m_cmd;
        uint8_t                       m_status;
        uint16_t                      m_opcode;
    };

};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

#pragma once

using std::unique_ptr;
using std::shared_ptr;

namespace hci
{
    class command;

    using hci_command = unique_ptr<command>;
    using hci_command_completion_cb_t = std::function<void(const hci_command&)>;

    class command
    {
    public:
        command( uint16_t ogf, uint16_t ocf ) : m_opcode(ogf << 10 | ocf)
        {
            m_cmd.put( m_opcode );
            m_cmd.put( (uint8_t) 0 ); // data length
        };

        uint8_t status() const { return m_status; }
        void set_completion_cb(hci_command_completion_cb_t cb) { m_cb = cb; }
        bytebuffer& buffer() { return m_cmd; };

    public:
        static hci_command reset();
        static hci_command le_set_event_mask( uint64_t le_event_mask );

    protected:

    protected:
        hci_command_completion_cb_t   m_cb;
        bytebuffer                    m_cmd;
        uint8_t                       m_status;
        uint16_t                      m_opcode;
    };

};

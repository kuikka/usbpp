#pragma once

#include <vector>
#include <list>
#include "libusbpp.h"

using std::unique_ptr;
using std::shared_ptr;

class bytebuffer
{
public:
    inline bytebuffer& put( uint8_t byte ) {
        m_buf.push_back( byte );
        return *this;
    };

    inline bytebuffer& put( uint16_t word ) {
        put( static_cast<uint8_t>( word & 0xff ) );
        put( static_cast<uint8_t>( word >> 8 ) );
        return *this;
    };

    inline bytebuffer& put( uint32_t dword ) {
        put( static_cast<uint16_t>( dword & 0xffff ) );
        put( static_cast<uint16_t>( dword >> 16 ) );
        return *this;
    };

    inline bytebuffer& put( uint64_t qword ) {
        put( static_cast<uint32_t>( qword & 0xffffffff ) );
        put( static_cast<uint32_t>( qword >> 32 ) );
        return *this;
    };

    inline uint8_t get_byte() {
        if ( m_read_index + sizeof(uint8_t) > m_buf.size() )
            return 0;
        return m_buf[m_read_index++];
    }

    inline uint16_t get_word() {
        if ( m_read_index + sizeof(uint16_t) > m_buf.size() )
            return 0;

        uint16_t val = m_buf[m_read_index++];
        val |= m_buf[m_read_index++] << 8;
        return val;
    }

    inline size_t length()
    {
        return m_buf.size();
    }

    inline uint8_t * ptr()
    {
        return m_buf.data();
    }

private:
    using data_type = std::vector<uint8_t>;
    data_type                m_buf;
    data_type::size_type     m_read_index = 0;
};


namespace hci
{
    class event
    {
    };

    class command;

    using hci_command = unique_ptr<command>;
    using event_cb_t = std::function<void(const hci::event&)>;
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


    class controller
    {
    public:
        using completed_status = bool;
        using completed_cb = std::function<void(completed_status)>;

    public:
        virtual void set_event_callback(event_cb_t cb)
        {
            m_event_callback = cb;
        }

        virtual void submit_command( hci_command cmd, hci_command_completion_cb_t cb )
        {
            cmd->set_completion_cb( cb );
            submit_command( std::move( cmd ) );
        }

        virtual void submit_command(hci_command cmd)
        {
            m_command_queue.push_back( std::move( cmd ) );
            send_next_command();
        }

        virtual void send_next_command()
        {
            if ( !m_command_ongoing && !m_command_queue.empty() )
            {
                auto &cmd = m_command_queue.front();
                send( cmd.get() );
//                m_command_ongoing = true;
            }
        }
        virtual bool reset(completed_cb);
        virtual bool init(completed_cb) = 0;
        virtual bool send(hci::command *cmd) = 0;

    protected:
        event_cb_t                 m_event_callback;
        std::list<hci_command>     m_command_queue;
        bool                       m_command_ongoing = false;
    };

    class usb_controller : public controller
    {
    public:
        usb_controller( usbpp::device& device );
        virtual bool init(controller::completed_cb) override;
        virtual bool send(hci::command *cmd) override;

    private:
        bool submit_event_transfer();
        void on_event(libusb_transfer_status,
            const uint8_t*, size_t);


    private:
        usbpp::device                   m_dev;
        unique_ptr<usbpp::handle>       m_handle;
        usbpp::endpoint                 m_event_in;
        usbpp::endpoint                 m_data_out;
        usbpp::endpoint                 m_data_in;
        std::unique_ptr<uint8_t[]>      m_event_buffer;
    };

    class controller_factory
    {
    };

    class manager
    {
    public:
        static manager& get()
        {
            static manager m;
            return m;
        }

        bool add( shared_ptr<hci::controller> c )
        {
            m_controllers.push_back( c );
            return true;
        }

    private:
        std::vector< shared_ptr<hci::controller> > m_controllers;
    };

    class usb_controller_factory : public controller_factory
    {
    public:
        usb_controller_factory( usbpp::context &ctx );
        void hotplug_cb_fn( usbpp::device device, bool added );
        void probe( usbpp::device& device );

    private:
        usbpp::context  &m_ctx;
        hci::manager    &m_manager;
    };

}

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

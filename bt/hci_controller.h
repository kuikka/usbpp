#pragma once

#include <vector>
#include <list>
#include "hci_event.h"
#include "hci.h"

using std::unique_ptr;
using std::shared_ptr;

namespace hci
{

    using event_cb_t = std::function<void(const hci::event&)>;
    using hci::hci_command;

    class controller
    {
    public:
        using completed_status = bool;
        using operation_completed_cb = std::function<void(completed_status)>;

    public:
        virtual void on_hci_event( const event &ev );
//        virtual void on_hci_data();

    public:
        virtual void set_event_callback(event_cb_t cb);
        virtual bool reset(operation_completed_cb);
        virtual bool init(operation_completed_cb) = 0;
        virtual bool send(hci::command *cmd) = 0;
        virtual void submit_command( hci_command cmd, hci_command_completion_cb_t cb );
        virtual void submit_command(hci_command cmd);
        const std::string& name() { return m_name; }

    protected:
        virtual void send_next_command();
        virtual void complete_command();

    protected:
        event_cb_t                 m_event_callback;
        std::list<hci_command>     m_command_queue;
	hci_command                m_current_command;
        bool                       m_command_ongoing = false;
        std::string                m_name;
    };

    class usb_controller : public controller
    {
    public:
        usb_controller( usbpp::usb_device& device );

    public:
        virtual bool init(controller::operation_completed_cb) override;
        virtual bool send(hci::command *cmd) override;

    private:
        bool submit_event_transfer();
        void on_event_transfer_completed(usbpp::usb_transfer_status,
            const uint8_t*, size_t);


    private:
        usbpp::usb_device                     m_dev;
        unique_ptr<usbpp::usb_device_handle>  m_handle;
        usbpp::usb_endpoint                   m_event_in;
        usbpp::usb_endpoint                   m_data_out;
        usbpp::usb_endpoint                   m_data_in;
        std::unique_ptr<uint8_t[]>            m_event_buffer;
    };

};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

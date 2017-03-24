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
        using completed_cb = std::function<void(completed_status)>;

    public:
        virtual void on_hci_event( const event &ev );
//        virtual void on_hci_data();

    public:
        virtual void set_event_callback(event_cb_t cb);
        virtual void submit_command( hci_command cmd, hci_command_completion_cb_t cb );
        virtual void submit_command(hci_command cmd);
        virtual void send_next_command();
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
        usb_controller( usbpp::usb_device& device );
        virtual bool init(controller::completed_cb) override;
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

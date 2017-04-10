#pragma once

#include <vector>
#include <list>
#include "usb/libusbpp.h"
#include "bytebuffer.h"

namespace hci
{
    class event
    {
    public:
        static const uint8_t COMMAND_COMPLETED = 0x0E;
        static const uint8_t COMMAND_STATUS    = 0x0F;
        static const uint8_t LE_META_EVENT     = 0x3E;
    public:
        event();
        ~event();
        event( const uint8_t *buffer, size_t length );

        int code() const { return m_event_code; };
        int length() const { return m_event_length; };
        const bytebuffer& data() const { return m_data; };

        bool parse( const uint8_t *buffer, size_t length );

    private:
        uint8_t    m_event_code;
        uint8_t    m_event_length;
        bytebuffer m_data;
    };

};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

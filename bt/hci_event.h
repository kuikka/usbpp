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
        event();
        ~event();
        event( const uint8_t *buffer, size_t length );

        int code() const { return m_event_code; };
        int length() const { return m_event_length; };

        bool parse( const uint8_t *buffer, size_t length );

    private:
        uint8_t    m_event_code;
        uint8_t    m_event_length;
        bytebuffer m_data;
    };

};

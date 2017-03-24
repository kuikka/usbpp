#include <iostream>
#include "hci.h"
#include "hexdump.h"

using std::cout;

namespace hci
{
    event::event()
    {}

    event::~event()
    {}

    event::event( const uint8_t *buffer, size_t length )
    {
        parse( buffer, length );
    }

    bool event::parse( const uint8_t *buffer, size_t length )
    {
        m_event_code = buffer[0];
        m_event_length = buffer[1];

        if ( length > 2 )
            m_data.set( buffer + 2, length - 2 );

        return true;
    }
};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

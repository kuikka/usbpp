#include <iostream>
#include <libusb.h>

#include <poll.h>
#include <cstdio>
#include <cstring>

#include "events.h"

namespace events {

    void event_base::on_event( int fd, short events )
    {
        auto it = m_receivers.find( fd );
        if ( it != m_receivers.end() )
        {
            short flags = std::get<1>( it->second );
            auto *r = std::get<0>( it->second );
            if (r && (events & flags))
                r->event_callback( fd, events );
        }
    }

    bool event_base::register_event( int fd, short flags, events::event_receiver *r )
    {
        printf("event_base::register_event %d %x %p\n", fd, flags, r);
        m_receivers[ fd ] = std::make_tuple( r, flags );
        return true;
    }

    void event_base::unregister_event( int fd, events::event_receiver *r )
    {
        auto i = m_receivers.find( fd );
        if ( i != m_receivers.end() )
        {
            auto p = (*i).second;
            if ( std::get<0>( p ) == r )
            {
                m_receivers.erase( i );
            }
        }
    }

    event_base::event_base()
    {
    }

    bool event_base::init()
    {
        return true;
    }

    void event_base::dispatch()
    {
        m_running = true;

        while(m_running)
        {
            const size_t nfds = m_receivers.size();

            struct pollfd fds[nfds];

            size_t i = 0;
            for ( const auto& p : m_receivers )
            {
                fds[i].fd = p.first;
                fds[i].events = std::get<1>( p.second );
                fds[i].revents = 0;
                i++;
            }

            int ret = ::poll( fds, nfds, 1000 );
            printf("Poll returned %d\n", ret);
            if ( ret > 0 )
            {
                for ( i = 0; i < nfds; ++i )
                {
                    if ( fds[i].revents )
                    {
printf("fd %d revents %x", fds[i].fd, fds[i].revents);
                        on_event( fds[i].fd, fds[i].revents );
                    }
                }
            }
        }
    };

};



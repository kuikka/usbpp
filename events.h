#pragma once

#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <list>
#include <functional>
#include <memory>

namespace events {

    class event_receiver
    {
    public:
        virtual void event_callback( int, short ) = 0;
    };

    class event_base
    {
    public:
        event_base();
        bool init();
        void dispatch();

    public:
        bool register_event( int fd, short flags, event_receiver *r );
        void unregister_event( int fd, event_receiver *r );

    protected:
        void on_event( int fd, short events );
        // fd => (event_receiver*, flags)
        std::map< int, std::tuple< event_receiver*, short > >	m_receivers;
        bool	m_running = false;
    };

};


#pragma once

#include <vector>
#include <list>
#include "usb/libusbpp.h"
#include "bytebuffer.h"
#include "hci_event.h"
#include "hci_command.h"
#include "hci_controller.h"

using std::unique_ptr;
using std::shared_ptr;

namespace hci
{
    class controller_factory;
    class controller;

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
            if ( m_controller_added_cb )
                m_controller_added_cb( *c );

            return true;
        }

        void add( hci::controller_factory * factory )
        {
            m_factories.push_back( factory );
        }

        void set_controller_added_cb( std::function< void(hci::controller&) > cb )
        {
            m_controller_added_cb = cb;
        };

    private:
        manager() {};
        std::vector< shared_ptr<hci::controller> > m_controllers;
        std::vector< hci::controller_factory* >    m_factories;
        std::function< void(hci::controller&) >    m_controller_added_cb;
    };

}

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

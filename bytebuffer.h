#pragma once

#include <vector>
#include <cstdint>

class bytebuffer
{
public:
    bytebuffer()
    {};

    bytebuffer( const uint8_t *data, size_t length )
      :   m_buf( data, data + length )
    {
    };

    void set( const uint8_t *data, size_t length )
    {
        m_buf = data_type( data, data + length );
    };

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

    inline uint8_t get_byte() const {
        if ( m_read_index + sizeof(uint8_t) > m_buf.size() )
            return 0;
        return m_buf[m_read_index++];
    }

    inline uint16_t get_word() const {
        if ( m_read_index + sizeof(uint16_t) > m_buf.size() )
            return 0;

        uint16_t val = m_buf[m_read_index++];
        val |= m_buf[m_read_index++] << 8;
        return val;
    }

    inline size_t length() const
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
    mutable data_type::size_type     m_read_index = 0;
};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

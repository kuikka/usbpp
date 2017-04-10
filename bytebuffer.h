#pragma once

#include <cstdint>
#include <cstring>

class bytebuffer
{
public:
    bytebuffer() {};
    ~bytebuffer()
    {
        if (m_own_buffer)
            delete[] m_buf;
    }

    bytebuffer(size_t length)
    {
        allocate(length);
    }

    // make a copy
    bytebuffer( const uint8_t *data, size_t length )
        :   bytebuffer(length)
    {
        memcpy(m_buf, data, length);
    };

    // make a copy from m_read_size onwards
    bytebuffer( const bytebuffer &bb, size_t length )
        :   bytebuffer(bb.m_buf + bb.m_read_index, length)
    {
    }

    void allocate(size_t length)
    {
        m_buf_size = length;
        m_buf = new uint8_t[m_buf_size];
        m_own_buffer = true;
    }

    // make a copy
    void set( const uint8_t *data, size_t length )
    {
        if (m_own_buffer && m_buf)
            delete[] m_buf;

        allocate(length);
        memcpy(m_buf, data, length);
    };

    inline bytebuffer& put( uint8_t byte ) {
        if (m_write_index < m_buf_size)
            m_buf[m_write_index++] = byte;
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
        if ( m_read_index >= m_buf_size )
            return 0;
        return m_buf[m_read_index++];
    }

    inline uint16_t get_word() const {
        if ( m_read_index >= m_buf_size )
            return 0;

        uint16_t val = m_buf[m_read_index++];
        val |= m_buf[m_read_index++] << 8;
        return val;
    }

    inline size_t length() const
    {
        return m_buf_size;
    }

    inline uint8_t * ptr()
    {
        return m_buf;
    }

    size_t remaining() const
    {
        return length() - m_read_index;
    }

    void wrap( uint8_t *data, size_t length )
    {
        if (m_own_buffer && m_buf)
        {
            delete[] m_buf;
            m_buf = nullptr;
        }

        m_own_buffer = false;
        m_read_index = m_write_index = 0;
        m_buf_size = length;
        m_buf = data;
    }

private:
//    using data_type = std::vector<uint8_t>;
//    data_type                m_buf;
//    mutable data_type::size_type     m_read_index = 0;
        bool    m_own_buffer = false;
        uint8_t *m_buf = nullptr;
        size_t  m_buf_size = 0;
        mutable size_t  m_read_index = 0;
        size_t  m_write_index = 0;
};

// vim: set shiftwidth=4 expandtab cinoptions=t0,g0,j1,ws,(s,W1:

#include <core/stream.h>

static Stream::EByteOrder byte_order() {
    union {
        uint8_t  char_value[2];
        uint16_t short_value;
    };
    char_value[0] = 1;
    char_value[1] = 0;

    if (short_value == 1)
        return Stream::ELittleEndian;
    else
        return Stream::EBigEndian;
}

const Stream::EByteOrder Stream::m_host_byte_order = ::byte_order();

Stream::Stream() : m_byte_order(m_host_byte_order) {}

Stream::~Stream() {}


#pragma once
#include <core/logger.h>
#include <core/object.h>
// =============================================================================
//                                  Helper Functions
// =============================================================================
template <typename T, std::enable_if_t<sizeof(T) == 1, int> = 0>
T swap(const T &v) {
    return v;
}

template <typename T, std::enable_if_t<sizeof(T) == 2, int> = 0>
T swap(const T &v) {
    return __builtin_bswap16(static_cast<uint16_t>(v));
}

template <typename T, std::enable_if_t<sizeof(T) == 4, int> = 0>
T swap(const T &v) {
    return __builtin_bswap32(static_cast<uint32_t>(v));
}

template <typename T, std::enable_if_t<sizeof(T) == 8, int> = 0>
T swap(const T &v) {
    return __builtin_bswap64(static_cast<uint64_t>(v));
}

struct Stream : public Object {
    enum EByteOrder { EBigEndian = 0, ELittleEndian = 1 };

    Stream();
    virtual ~Stream();

    virtual void close() = 0;

    virtual void read(void *p, size_t size) = 0;

    template <typename T>
    void read(T &value) {
        read(&value, sizeof(T));
        if (needs_endianness_swap()) {
            value = swap(value);
        }
    }

    template <typename T>
    T read() {
        T value;
        read(value);
        return value;
    }

    template <typename T>
    void read_array(T *value, size_t count) {
        read(value, sizeof(T) * count);
        if (needs_endianness_swap()) {
            for (size_t i = 0; i < count; ++i)
                value[i] = swap(value[i]);
        }
    }

    /// Returns true if we need to perform endianness swapping before writing or
    /// reading.
    bool needs_endianness_swap() const {
        return m_byte_order != m_host_byte_order;
    }

protected:
    /// Copying is disallowed.
    Stream(const Stream &)         = delete;
    void operator=(const Stream &) = delete;

private:
    static const EByteOrder m_host_byte_order;
    EByteOrder              m_byte_order;
};

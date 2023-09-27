#include <core/fstream.h>

inline std::ios::openmode ios_flag(FileStream::EMode mode) {
    switch (mode) {
        case FileStream::ERead:
            return std::ios::binary | std::ios::in;
        case FileStream::EReadWrite:
            return std::ios::binary | std::ios::in | std::ios::out;
        case FileStream::ETruncReadWrite:
            return std::ios::binary | std::ios::in | std::ios::out |
                   std::ios::trunc;
        default:
            Throw("Internal error");
    }
}

FileStream::FileStream(const fs::path &p, EMode mode)
    : Stream(), m_mode(mode), m_path(p),
      m_file(new std::fstream(p, ::ios_flag(mode))) {

    if (!m_file->good())
        Throw("\"%s\": I/O error while attempting to open file: %s",
              m_path.string(), strerror(errno));
}

FileStream::~FileStream() { close(); }

void FileStream::close() { m_file->close(); };

void FileStream::read(void *p, size_t size) {
    m_file->read((char *) p, size);

    if (!m_file->good()) {
        bool   eof    = m_file->eof();
        size_t gcount = m_file->gcount();
        m_file->clear();
        if (eof)
            Throw("\"%s\": read %zu out of %zu bytes", m_path.string(), gcount,
                  size);
        else
            Throw("\"%s\": I/O error while attempting to read %zu bytes: %s",
                  m_path.string(), size, strerror(errno));
    }
}
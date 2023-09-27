#pragma once

#include <core/fwd.h>
#include <core/logger.h>
#include <core/stream.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

struct FileStream : public Stream {
    enum EMode {
        /// Opens a file in (binary) read-only mode
        ERead,
        /// Opens (but never creates) a file in (binary) read-write mode
        EReadWrite,
        /// Opens (and truncates) a file in (binary) read-write mode
        ETruncReadWrite
    };

    FileStream(const fs::path &p, EMode mode = ERead);
    virtual ~FileStream();

    virtual void close() override;

    virtual void read(void *p, size_t size) override;

    std::string type_name() const override { return "FileStream"; }

protected:
private:
    EMode                         m_mode;
    fs::path                      m_path;
    std::unique_ptr<std::fstream> m_file;
};
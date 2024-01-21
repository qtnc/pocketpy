#pragma once

#include "cffi.h"

namespace pkpy{
    unsigned char* _default_import_handler(VM*, const char*, int, int*);
extern struct IOHandler *_default_io_handler;
    void add_module_os(VM* vm);
    void add_module_io(VM* vm);
}

#include <cstdio>

namespace pkpy{

struct IOHandler {
virtual bool exists (const std::string& name) = 0;
virtual bool isfile (const std::string& name) = 0;
virtual bool isdir (const std::string& name) = 0;
virtual void* open (const std::string& name, const std::string& mode) = 0;
virtual void seek (void* fp, int n, int a) = 0;
virtual int tell (void* fp) = 0;
virtual std::size_t read (void* buffer, int size, int count, void* fp) = 0;
virtual std::size_t write (const void* buffer, int size, int count, void* fp) = 0;
virtual void close (void* fp) = 0;

virtual void listdir (const std::string& path, VM* vm, List& list) = 0;
virtual bool remove (const std::string& name) = 0;
virtual bool mkdir (const std::string& name) = 0;
virtual bool rmdir (const std::string& path) = 0;
};

#if PK_ENABLE_OS

struct FileIO {
    PY_CLASS(FileIO, io, FileIO)

    Str file;
    Str mode;
    void* fp;

    bool is_text() const { return mode != "rb" && mode != "wb" && mode != "ab"; }
    FileIO(VM* vm, std::string file, std::string mode);
    void close(VM* vm);
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

#endif

} // namespace pkpy

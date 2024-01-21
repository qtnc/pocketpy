#include "pocketpy/io.h"
#include "pocketpy/common.h"
#include<dirent.h>
#include<cstdio>

namespace pkpy{

#if PK_ENABLE_OS
static FILE* io_fopen(const char* name, const char* mode){
#if _MSC_VER
    FILE* fp;
    errno_t err = fopen_s(&fp, name, mode);
    if(err != 0) return nullptr;
    return fp;
#else
    return fopen(name, mode);
#endif
}

static size_t io_fread(void* buffer, size_t size, size_t count, FILE* fp){
#if _MSC_VER
    return fread_s(buffer, std::numeric_limits<size_t>::max(), size, count, fp);
#else
    return fread(buffer, size, count, fp);
#endif
}
#endif


unsigned char* _default_import_handler(VM* vm, const char* name_p, int name_size, int* out_size){
#if PK_ENABLE_OS
    std::string name(name_p, name_size);
    bool exists = vm->_io_handler->exists(name);
    if(!exists) return nullptr;
    void* fp = vm->_io_handler->open(name, "rb");
    if(!fp) return nullptr;
    vm->_io_handler->seek(fp, 0, SEEK_END);
    int buffer_size = vm->_io_handler->tell(fp);
    unsigned char* buffer = new unsigned char[buffer_size];
    vm->_io_handler->seek(fp, 0, SEEK_SET);
    size_t sz = vm->_io_handler->read(buffer, 1, buffer_size, fp);
    PK_UNUSED(sz);
    vm->_io_handler->close(fp);
    *out_size = buffer_size;
    return buffer;
#else
    return nullptr;
#endif
};

struct DefaultIOHandler: IOHandler {
#if PK_ENABLE_OS
bool exists (const std::string& name) override {
return isfile(name) || isdir(name);
}
bool isfile (const std::string& name) override {
auto fp = io_fopen(name.c_str(), "rb");
if (fp) fclose(fp);
return !!fp;
}
bool isdir (const std::string& name) override {
auto dp = opendir(name.c_str());
if (dp) closedir(dp);
return !!dp;
}
void* open (const std::string& name, const std::string& mode) override {
return io_fopen(name.c_str(), mode.c_str());
}
void seek (void* fp, int n, int a) override {
if (fp) fseek((FILE*)fp, n, a);
}
int tell (void* fp) override {
return fp? ftell((FILE*)fp) : -1;
}
std::size_t read (void* buffer, int size, int count, void* fp) override {
return fp? io_fread(buffer, size, count, (FILE*)fp) : -1;
}
std::size_t write (const void* buffer, int size, int count, void* fp) override {
return fp? fwrite(buffer, size, count, (FILE*)fp) : -1;
}
void close (void* fp) override {
if (fp) fclose((FILE*)fp);
}
void listdir (const std::string& path, VM* vm, List& list) override {
auto dp = opendir(path.c_str());
if (!dp) return;
while(auto d = readdir(dp)) {
std::string name = d->d_name;
if (name.empty() || name=="." || name=="..") continue;
list.push_back(VAR(name));
}
closedir(dp);
}
bool remove (const std::string& name) override {
return ::remove(name.c_str());
}
bool mkdir (const std::string& name) override {
return ::mkdir(name.c_str());
}
bool rmdir (const std::string& name) override {	
return ::rmdir(name.c_str());
}
#else
bool exists (const std::string& name) override { return false; }
bool isfile (const std::string& name) override { return false; }
bool isdir (const std::string& name) override { return false; }
void* open (const std::string& name, const std::string& mode) override { return nullptr; }
void seek (void* fp, int n, int a) override { }
int tell (void* fp) override { return -1; }
std::size_t read (void* buffer, int size, int count, void* fp) override { return -1; }
std::size_t write (const void* buffer, int size, int count, void* fp) override { return -1; }
void close (void* fp) override { }
void listdir (const std::string& path, VM* vm, List& list) override {}
bool remove (const std::string& name) override { return false; }
bool mkdir (const std::string& name) override { return false; }
bool rmdir (const std::string& path) override { return false; }
#endif
}; // DefaultIOHandler

static DefaultIOHandler _default_io_handler_1;
IOHandler* _default_io_handler = &_default_io_handler_1;

#if PK_ENABLE_OS
    void FileIO::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<FileIO>(cls, vm,
                       py_cast<Str&>(vm, args[1]).str(),
                       py_cast<Str&>(vm, args[2]).str());
        });

        vm->bind_method<0>(type, "read", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            vm->_io_handler->seek(io.fp, 0, SEEK_END);
            int buffer_size = vm->_io_handler->tell(io.fp);
            unsigned char* buffer = new unsigned char[buffer_size];
            vm->_io_handler->seek(io.fp, 0, SEEK_SET);
            size_t actual_size = vm->_io_handler->read(buffer, 1, buffer_size, io.fp);
            PK_ASSERT(actual_size <= buffer_size);
            // in text mode, CR may be dropped, which may cause `actual_size < buffer_size`
            Bytes b(buffer, actual_size);
            if(io.is_text()) return VAR(b.str());
            return VAR(std::move(b));
        });

        vm->bind_method<1>(type, "write", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            if(io.is_text()){
                Str& s = CAST(Str&, args[1]);
                vm->_io_handler->write(s.data, 1, s.length(), io.fp);
            }else{
                Bytes& buffer = CAST(Bytes&, args[1]);
                vm->_io_handler->write(buffer.data(), 1, buffer.size(), io.fp);
            }
            return vm->None;
        });

        vm->bind_method<0>(type, "close", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            io.close(vm);
            return vm->None;
        });

        vm->bind_method<0>(type, "__exit__", [](VM* vm, ArgsView args){
            FileIO& io = CAST(FileIO&, args[0]);
            io.close(vm);
            return vm->None;
        });

        vm->bind_method<0>(type, "__enter__", PK_LAMBDA(args[0]));
    }

    FileIO::FileIO(VM* vm, std::string file, std::string mode): file(file), mode(mode) {
        fp = vm->_io_handler->open(file, mode);
        if(!fp) vm->IOError(strerror(errno));
    }

    void FileIO::close(VM* vm){
        if(fp == nullptr) return;
        vm->_io_handler->close(fp);
        fp = nullptr;
    }

#endif

void add_module_io(VM* vm){
#if PK_ENABLE_OS
    PyObject* mod = vm->new_module("io");
    FileIO::register_class(vm, mod);
    vm->bind(vm->builtins, "open(path, mode='r')", [](VM* vm, ArgsView args){
        PK_LOCAL_STATIC StrName m_io("io");
        PK_LOCAL_STATIC StrName m_FileIO("FileIO");
        return vm->call(vm->_modules[m_io]->attr(m_FileIO), args[0], args[1]);
    });
#endif
}

void add_module_os(VM* vm){
#if PK_ENABLE_OS
    PyObject* mod = vm->new_module("os");
    PyObject* path_obj = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    mod->attr().set("path", path_obj);
    vm->bind_func<1>(mod, "listdir", [](VM* vm, ArgsView args){
std::string path = CAST(Str&, args[0]).str();
        try{
        List ret;
vm->_io_handler->listdir(path, vm, ret);
        return VAR(ret);
        }catch(std::exception& e){
            std::string msg = e.what();
            auto pos = msg.find_last_of(":");
            if(pos != std::string::npos) msg = msg.substr(pos + 1);
            vm->IOError(Str(msg).lstrip());
        }
return vm->None;
    });

    vm->bind_func<1>(mod, "remove", [](VM* vm, ArgsView args){
std::string path = CAST(Str&, args[0]).str();
        bool ok = vm->_io_handler->remove(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "mkdir", [](VM* vm, ArgsView args){
std::string path = CAST(Str&, args[0]).str();
        bool ok = vm->_io_handler->mkdir(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(mod, "rmdir", [](VM* vm, ArgsView args){
std::string path = CAST(Str&, args[0]).str();
        bool ok = vm->_io_handler->rmdir(path);
        if(!ok) vm->IOError("operation failed");
        return vm->None;
    });

    vm->bind_func<1>(path_obj, "exists", [](VM* vm, ArgsView args){
std::string path = CAST(Str&, args[0]).str();
        bool exists = vm->_io_handler->exists(path);
        return VAR(exists);
    });

    vm->bind_func<1>(path_obj, "isfile", [](VM* vm, ArgsView args){
std::string path = CAST(Str&, args[0]).str();
        bool re = vm->_io_handler->isfile(path);
        return VAR(re);
    });

    vm->bind_func<1>(path_obj, "isdir", [](VM* vm, ArgsView args){
std::string path = CAST(Str&, args[0]).str();
        bool re = vm->_io_handler->isdir(path);
        return VAR(re);
    });

        CodeObject_ code = vm->compile(kPythonLibs["os"], "os.py", EXEC_MODE);
        vm->_exec(code, mod);
#endif
}

}   // namespace pkpy
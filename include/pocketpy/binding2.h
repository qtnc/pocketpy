#pragma once
#include "obj.h"
#include "cffi.h"
#include "vm.h"
#include "bindings.h"

namespace pkpy {

template <class T, class... A>
struct CtorBinder { 
template<std::size_t... I>
static PyObject* call (VM* vm,  ArgsView args, std::index_sequence<I...> seq) {
return vm->heap.gcnew<T>(T::_type(vm), py_cast<typename std::tuple_element<I, std::tuple<A...>>::type>(vm, args[I+1])... );
}
static inline void bind (VM* vm, PyObject* type, const char* name) {
vm->bind(type, name, [](VM* vm, ArgsView args){
typedef std::make_index_sequence<sizeof...(A)> seq;
return call(vm, args, seq());
});
}
};//end

struct Binder {
VM* vm;
PyObject* mod;
PyObject* obj;

inline Binder (VM* vm1, PyObject* mod1, PyObject* obj1): vm(vm1), mod(mod1), obj(obj1) {}

template<typename Ret, typename... Params>
inline void bind (const char* sig, Ret(*func)(Params...), const char* doc = nullptr){
    auto proxy = new NativeProxyFuncC<Ret, Params...>(func);
    vm->bind(obj, sig, doc, proxy_wrapper, proxy);
}

template<typename Ret, typename T, typename... Params>
inline void bind (const char* sig, Ret(T::*func)(Params...), const char* doc = nullptr){
    auto proxy = new NativeProxyMethodC<Ret, T, Params...>(func);
    vm->bind(obj, sig, doc, proxy_wrapper, proxy);
}

template<class T, class... Params>
inline void bindConstructor (const char* sig) {
CtorBinder<T, Params...>::bind(vm, obj, sig);
}

template<class T>
void bind (const char* name, T* prop) {
vm->bind_property(obj, name,
[](VM* vm, ArgsView args){
T* prop = lambda_get_userdata<T*>(args.begin());
return py_var(vm, *prop);
},
[](VM* vm, ArgsView args){
T* prop = lambda_get_userdata<T*>(args.begin());
*prop = py_cast<T>(vm, args[1]);
return vm->None;
},
prop, prop);
}

template<class T, class P>
void bind (const char* name, P T::*prop) {
typedef P T::*Prop;
vm->bind_property(obj, name,
[](VM* vm, ArgsView args){
Prop prop = lambda_get_userdata<Prop>(args.begin());
T& self = _py_cast<T&>(vm, args[0]);
return py_var(vm, self.*prop);
},
[](VM* vm, ArgsView args){
Prop prop = lambda_get_userdata<Prop>(args.begin());
T& self = _py_cast<T&>(vm, args[0]);
self.*prop = py_cast<P>(vm, args[1]);
return vm->None;
},
prop, prop);
}

template <class T>
inline void bind (const char* name, const T& val) {
    obj->attr().set(name, VAR(val));
}

}; // Binder

} // namespace pkpy

#define PY_REG(CPPNAME, MODNAME, PYNAME) \
PY_CLASS(CPPNAME, MODNAME, PYNAME) \
    static inline void _register (pkpy::VM* vm, pkpy::PyObject* mod, pkpy::PyObject* type) { _register2(Binder(vm, mod, type)); } \
    static void _register2 (Binder binder)





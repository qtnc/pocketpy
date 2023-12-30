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

template<class B = void>
struct Binder {
VM* vm;
PyObject* mod;
PyObject* obj;

inline Binder (VM* vm1, PyObject* mod1, PyObject* obj1 = nullptr): vm(vm1), mod(mod1), obj(obj1?obj1:mod1) {}

template<typename Ret, typename... Params>
inline Binder& bind (const char* sig, Ret(*func)(Params...), const char* doc = nullptr){
    auto proxy = new NativeProxyFuncC<Ret, Params...>(func);
    vm->bind(obj, sig, doc, proxy_wrapper, proxy);
return *this;
}

template<typename T, typename Ret, typename... Params>
inline Binder& bind (const char* sig, Ret(T::*func)(Params...), const char* doc = nullptr){
typedef Ret(B::*BF)(Params...);
    auto proxy = new NativeProxyMethodC<Ret, B, Params...>( (BF) func);
    vm->bind(obj, sig, doc, proxy_wrapper, proxy);
return *this;
}

inline Binder& bindFunc (const char* sig, NativeFuncC func, const char* doc = nullptr) {
vm->bind(obj, sig, doc, func);
return *this;
}

template<class T, class... Params>
inline Binder& bindCtor (const char* sig) {
CtorBinder<T, Params...>::bind(vm, obj, sig);
return *this;
}

template<class T>
Binder& bindStaticProp (const char* name, T* prop) {
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
return *this;
}

template<class T, class P>
Binder& bindProp (const char* name, P T::*prop) {
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
return *this;
}

template<class T, class GP, class SP, class SR>
Binder& bindProp (const char* name, GP(T::*getter)(), SR(T::*setter)(SP)) {
    auto proxyGetter = getter? new NativeProxyMethodC<GP, T>(getter) :nullptr;
    auto proxySetter = setter? new NativeProxyMethodC<SR, T, SP>(setter) :nullptr;
vm->bind_property(obj, name, getter?proxy_wrapper:nullptr, setter?proxy_wrapper:nullptr, proxyGetter, proxySetter);
return *this;
}

template<class T, class GP>
Binder& bindProp (const char* name, GP(T::*getter)()) {
return bindProp<T, GP, int, int>(name, getter, nullptr);
}

template<class GP, class SP, class SR>
Binder& bindProp (const char* name, GP(*getter)(), SR(*setter)(SP)) {
    auto proxyGetter = getter? new NativeProxyFuncC<GP>(getter) :nullptr;
    auto proxySetter = setter? new NativeProxyFuncC<SR, SP>(setter) :nullptr;
vm->bind_property(obj, name, getter?proxy_wrapper:nullptr, setter?proxy_wrapper:nullptr, proxyGetter, proxySetter);
return *this;
}

template<class GP>
Binder& bindProp (const char* name, GP(*getter)()) {
return bindProp<GP, int, int>(name, getter, nullptr);
}

Binder& bindPropFunc (const char* name, NativeFuncC  getter, NativeFuncC  setter = nullptr) {
vm->bind_property(obj, name, getter, setter);
return *this;
}

template <class T>
inline Binder& bindValue (const char* name, const T& val) {
    obj->attr().set(name, VAR(val));
return *this;
}

Binder& bindValue (const char* name, PyObject* val) {
    obj->attr().set(name, val);
return *this;
}

#define OP(NAME) template<class F> Binder& op_##NAME (const F& func) { \
vm->bind__##NAME##__(PK_OBJ_GET(Type, obj), func); \
return *this; \
}
OP(add) OP(sub) 
OP(mul) OP(matmul)
OP(truediv) OP(floordiv)
OP(mod) OP(pow)
OP(eq) OP(lt) OP(le) OP(gt) OP(ge)
OP(rshift) OP(lshift)
OP(and) OP(or) OP(xor)
OP(getitem) OP(setitem) OP(delitem) OP(contains)
OP(repr) OP(str)
OP(iter) OP(next)
OP(invert) OP(neg)
OP(len) OP(bool) OP(hash)
#undef OP

}; // Binder

template<> inline std::string py_cast<std::string> (VM* vm, PyObject* obj) {
return py_cast<Str&>(vm, obj) .str();
}

template<> inline const std::string& py_cast<const std::string&> (VM* vm, PyObject* obj) {
static std::string holder[16];
static int i = 0;
std::string& h = holder[++i%16];
h = py_cast<Str&>(vm, obj) .str();
return h;
}

template <class T, class Z = typename std::enable_if<is_py_class<T>::value>::type>
    inline PyObject* py_var(VM* vm, const T& value) { 
return vm->heap.gcnew<T>(T::_type(vm), value);
}  

template <class T, class Z = typename std::enable_if<is_py_class<T>::value>::type>
    inline PyObject* py_var(VM* vm, T&& value) { 
return vm->heap.gcnew<T>(T::_type(vm), std::move(value));
} 

template<class T>
inline bool is_type (VM* vm, PyObject* obj) {
using Tp = std::decay_t<T>;
if constexpr (is_py_class<Tp>::value) return is_type(obj, Tp::_type(vm));
else if constexpr (std::is_integral<Tp>::value) return is_int(obj);
else if constexpr (std::is_enum<Tp>::value) return is_int(obj);
else if constexpr (std::is_floating_point<Tp>::value) return is_float(obj);
#define TP(CTYPE,PTYPE) else if constexpr (std::is_same<Tp, CTYPE>::value) return is_type(obj, vm->PTYPE);
TP(std::string, tp_str)
TP(Str, tp_str)
TP(bool, tp_bool)
TP(List, tp_list)
TP(Dict, tp_dict)
TP(Tuple, tp_tuple)
TP(Bytes, tp_bytes)
TP(Slice, tp_slice)
#undef TP
else return false;
}

template<class T>
inline bool try_py_cast (VM* vm, PyObject* obj, T& re) {
if (!is_type<T>(vm, obj)) return false;
re = py_cast<T>(vm, obj);
return true;
}

template<class T>
inline bool try_py_cast (VM* vm, PyObject* obj, T*& re) {
if (!is_type<T>(vm, obj)) return false;
re = &py_cast<T&>(vm, obj);
return true;
}

inline PyObject* py_create_dummy (VM* vm) {
return vm->heap.gcnew<DummyInstance>(vm->tp_object);
}

template<class T, class... A>
inline PyObject* py_create (VM* vm, A&&... args) {
return vm->heap.gcnew<T>(T::_type(vm), std::forward<A>(args)...);
}

} // namespace pkpy

#define PY_REG(CPPNAME, MODNAME, PYNAME) \
PY_CLASS(CPPNAME, MODNAME, PYNAME) \
    static inline void _register (pkpy::VM* vm, pkpy::PyObject* mod, pkpy::PyObject* type) { _register(Binder<CPPNAME>(vm, mod, type)); } \
    static void _register (Binder<CPPNAME> binder)





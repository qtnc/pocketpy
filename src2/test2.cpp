#include<iostream>
#include<sstream>
    #include "pocketpy.h"
#include "pocketpy/binding2.hpp"
#include<clocale>
#include<cstdio>
using namespace pkpy;

struct Point {
double x, y;

Point () = default;
Point (double a=0, double b=0): x(a), y(b) {}
double length () { return sqrt(x*x+y*y); }
std::string repr () {
std::ostringstream out;
out << "Point(" << x << ", " << y << ")";
return out.str();
}
void print (const std::string& s) {
std::cout << s << ' ' << repr() << std::endl;
}
Point operator+ (const Point& p) {
std::cout << std::boolalpha << "this=" << (void*)this << ", other=" << (void*)&p << ", same=" << (this==&p) << std::endl;
return Point(x+p.x, y+p.y);
}
Point& operator+= (const Point& p) {
std::cout << std::boolalpha << "this=" << (void*)this << ", other=" << (void*)&p << ", same=" << (this==&p) << std::endl;
x+=p.x; y+=p.y;
return *this;
}
int getX () { return x; }
void setX (int i) { x=i; }

PY_REG(Point) {
binder
.bindValue("half", 0.5)
.bindCtor<Point, double, double>("__new__ (cls, x=0, y=0)")
.bindProp("x", &Point::x)
.bindProp("y", &Point::y)
.bindProp("xb", &getX, &setX)
.bind("__len__(self)", &length)
.bind("__add__(self,other)", &operator+)
.bind("append(self,other)", &operator+=)
.bind("print(self,s)", &print)
.op_repr([](VM* vm, PyObject* self){ 
Point& pt = CAST(Point&, self);
return pkpy::Str(pt.repr());
});
;
}

};//Point

bool ptnonull (Point* p) {
if (p) std::cout << "Passed " << p->repr() << std::endl;
else std::cout << "Passed nullptr" << std::endl;
return p && p->length()>0;
}

void regpt (void* pvm) {
VM* vm = (VM*)pvm;
vm->register_user_class<Point>(vm->builtins, "Point");

PyObject* dummy = py_create_dummy(vm);

Binder(vm, vm->_main)
.bindValue("dummy", dummy)
.bind("nonempty(p=None)", &ptnonull)
.bindFunc("testf(o=None)", [](VM* vm, ArgsView args){
if (int i; try_py_cast(vm, args[0], i)) return py_var(vm, "Int: " + std::to_string(i));
else if (std::string s; try_py_cast(vm, args[0], s)) return py_var(vm, "String: " +s);
else if (Point* p; try_py_cast(vm, args[0], p)) return py_var(vm, p->repr());
else return py_var(vm, "Something else");
});

printf("Locale = %s\n", setlocale(LC_ALL, "French_France.1252"));
}

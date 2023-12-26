#include<iostream>
    #include "pocketpy.h"
#include "pocketpy/binding2.h"
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

PY_REG(Point, builtins, Point) {
binder
.bind("half", 0.5)
.bindConstructor<Point, double, double>("__new__ (cls, x=0, y=0)")
.bind("x", &Point::x)
.bind("y", &Point::y)
.bind("__len__(self)", &length)
.bind("__repr__(self)", &repr)
.bind("__add__(self,other)", &operator+)
.bind("append(self,other)", &operator+=)
.bind("print(self,s)", &print)
;
}

};//Point
///PY_CLASS_CONV(Point)

void regpt (void* pvm) {
VM* vm = (VM*)pvm;
Point::register_class(vm, vm->builtins);

Binder(vm, vm->_main)
.bindf("testf(o)", [](VM* vm, ArgsView args){
if (int i; try_py_cast(vm, args[0], i)) return py_var(vm, "Int: " + std::to_string(i));
else if (std::string s; try_py_cast(vm, args[0], s)) return py_var(vm, "String: " +s);
else if (Point* p; try_py_cast(vm, args[0], p)) return py_var(vm, p->repr());
else return py_var(vm, "Something else");
});

}

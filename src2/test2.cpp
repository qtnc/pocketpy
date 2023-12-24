#include<iostream>
    #include "pocketpy.h"
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

PY_CLASS(Point, builtins, Point)
    static void _register(VM* vm, PyObject* mod, PyObject* type) {
vm->bindcp(type, "half", 0.5);
vm->bindc<Point, double, double>(type, "__new__ (cls, x=0, y=0)");
vm->bindp(type, "x", &Point::x);
vm->bindp(type, "y", &Point::y);
vm->bindf(type, "__repr__(self)", &repr);
vm->bindf(type, "__len__(self)", &length);
vm->bindf(type, "__add__(self,other)", &operator+);
vm->bindf(type, "append(self,other)", &operator+=);
//vm->bindf(type, "print(self,s)", &print);
}

};//Point
PY_CLASS_CONV(Point)

void testz (VM* vm, PyObject* obj) {
//std::string s = py_cast<std::string>(vm, obj);
}

void regpt (void* pvm) {
VM* vm = (VM*)pvm;
Point::register_class(vm, vm->builtins);
}

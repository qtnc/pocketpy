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
binder.bind("half", 0.5);
binder.bindConstructor<Point, double, double>("__new__ (cls, x=0, y=0)");
binder.bind("x", &Point::x);
binder.bind("y", &Point::y);
binder.bind("__len__(self)", &length);
binder.bind("__repr__(self)", &repr);
binder.bind("__add__(self,other)", &operator+);
binder.bind("append(self,other)", &operator+=);
//binder.bind("print(self,s)", &print);
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

#pragma once

#include "cffi.h"
#include<random>

namespace pkpy{

struct Random{
    PY_CLASS(Random, random, Random)
    std::mt19937 gen;

Random ();
static void _register(VM* vm, PyObject* mod, PyObject* type);
static PyObject* instance;
};

void add_module_random(VM* vm);

} // namespace pkpy
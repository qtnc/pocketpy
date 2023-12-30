#include<clocale>
#include<cstdlib>

double strtod_c (const char* s, char** endptr = nullptr) {
static _locale_t c = _create_locale(LC_ALL, "C");
return _strtod_l(s, endptr, c);
}

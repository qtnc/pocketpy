#include<clocale>
#include<cstdlib>

double strtod_c (const char* s, char** e) {
static _locale_t cLocale;
static bool init = false;
if (!init) {
cLocale = _create_locale(LC_ALL, "c");
init = true;
}
return _strtod_l(s, e, cLocale);
}

LIBNAME=PocketPy
DEFINES=$(options)
SRCS=$(wildcard src/*.cpp)
PY_SRCS=$(wildcard python/*.py)

DEFINES += PK_ENABLE_OS=0

ifeq ($(OS),Windows_NT)
EXT_EXE=.exe
EXT_LIB=.dll
LDFLAGS=-Wl,--add-stdcall-alias -Wl,--enable-stdcall-fixup
else
EXT_EXE=
EXT_LIB=.so
LDFLAGS=
endif

ifeq ($(mode),release)
NAME_SUFFIX=
DEFINES += RELEASE
CXXOPTFLAGS=-s -O3
else
NAME_SUFFIX=d
DEFINES += DEBUG
CXXOPTFLAGS=-g
endif

LIBRARY=$(LIBNAME)$(NAME_SUFFIX)$(EXT_LIB)
OBJDIR=obj$(NAME_SUFFIX)/

CXX=g++
CXXFLAGS=-std=gnu++17 $(addprefix -D,$(DEFINES)) -mthreads -Iinclude
LDFLAGSB=-shared -Wl,--out-implib,lib$(LIBNAME)$(NAME_SUFFIX).a $(LDFLAGS)

OBJS=$(addprefix $(OBJDIR),$(SRCS:.cpp=.o))
PERCENT=%

all: $(LIBRARY)

.PHONY: $(LIBRARY)

clean:
	rm -r $(OBJDIR)

$(LIBRARY): $(OBJS)
		mkdir.exe -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXOPTFLAGS) -o $@ $^ $(LDFLAGSB)

$(OBJDIR)%.o: %.cpp $(wildcard include/pocketpy/%.h) include/pocketpy/_generated.h
	$(CXX) $(CXXFLAGS) $(CXXOPTFLAGS) -c -o $@ $<

include/pocketpy/_generated.h: $(PY_SRCS)
	python prebuild.py


LIBNAME=libpocketpy
EXENAME=pocketpy
DEFINES=$(options)
SRCS=$(wildcard src/*.cpp)
EXE_SRCS=$(wildcard src2/*.cpp)
PY_SRCS=$(wildcard python/*.py)

ifeq ($(OS),Windows_NT)
EXT_EXE=.exe
EXT_LIB=.a
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
EXECUTABLE=$(EXENAME)$(NAME_SUFFIX)$(EXT_EXE)
OBJDIR=obj$(NAME_SUFFIX)/

CXX=g++
CXXFLAGS=-std=gnu++17 $(addprefix -D,$(DEFINES)) -mthreads -Iinclude

OBJS=$(addprefix $(OBJDIR),$(SRCS:.cpp=.o))
EXE_OBJS=$(addprefix $(OBJDIR),$(EXE_SRCS:.cpp=.o))
PERCENT=%

all: $(EXECUTABLE)

.PHONY: $(EXECUTABLE)

clean:
	rm -r $(OBJDIR)

$(LIBRARY): $(OBJS)
	ar ru $@ $^

$(EXECUTABLE): $(LIBRARY) $(EXE_OBJS)
	$(CXX) $(CXXFLAGS) $(CXXOPTFLAGS) -o $@ $^ -L. $l$<

$(OBJDIR)%.o: %.cpp $(wildcard include/pocketpy/%.h) include/pocketpy/_generated.h
	mkdir.exe -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXOPTFLAGS) -c -o $@ $<

include/pocketpy/_generated.h: $(PY_SRCS)
	python prebuild.py


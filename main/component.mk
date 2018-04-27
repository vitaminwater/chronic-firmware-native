#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

ARDUINO_CORE_LIBS := $(patsubst $(COMPONENT_PATH)/%,%,$(sort $(dir $(wildcard $(COMPONENT_PATH)/../components/*/*/))))

CXXFLAGS+=-std=c++11
COMPONENT_ADD_LDFLAGS=-lstdc++ -l$(COMPONENT_NAME)
COMPONENT_ADD_INCLUDEDIRS:=$(ARDUINO_CORE_LIBS)

#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
CXXFLAGS+=-std=c++11
COMPONENT_ADD_LDFLAGS=-lstdc++ -l$(COMPONENT_NAME)
COMPONENT_ADD_INCLUDEDIRS+=../components/BLE
COMPONENT_ADD_INCLUDEDIRS+=../components/WiFi
COMPONENT_ADD_INCLUDEDIRS+=../components/WiFiClientSecure
COMPONENT_ADD_INCLUDEDIRS+=../components/IPAddress

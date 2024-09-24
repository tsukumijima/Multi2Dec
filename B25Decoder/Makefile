SRCS := \
  BonProject__BcasCardReader.cpp \
  BonProject__BonDebug.cpp \
  BonProject__BonObject.cpp \
  BonProject__MediaBase.cpp \
  BonProject__MediaGrabber.cpp \
  BonProject__Multi2Decoder.cpp \
  BonProject__MyWinScard.cpp \
  BonProject__SyncObject.cpp \
  BonProject__TsDataEncode.cpp \
  BonProject__TsDescBase.cpp \
  BonProject__TsDescrambler.cpp \
  BonProject__TsDescriptor.cpp \
  BonProject__TsPacket.cpp \
  BonProject__TsPacketSync.cpp \
  BonProject__TsPidMapper.cpp \
  BonProject__TsSection.cpp \
  BonProject__TsTable.cpp \
  BonProject__TsTableBase.cpp \
  B25Decoder.cpp

ALL_H := \
  $(wildcard ../BonSDK/Include/*.h) \
  $(wildcard BonProject/*.h) \
  B25Decoder.h \
  IB25Decoder.h \
  stdafx.h

PKG_CONFIG_RESULT := $(shell pkg-config libpcsclite --cflags)
ifeq ($(PKG_CONFIG_RESULT),)
  $(error Failed)
endif
CPPFLAGS := $(PKG_CONFIG_RESULT) -I. -I../BonSDK/Include -I./BonProject -DNDEBUG -DMINIMIZE_FOR_B25DECODER $(CPPFLAGS)
CXXFLAGS := -std=c++17 -Wall -pedantic-errors -fPIC -pthread $(CXXFLAGS)
LDFLAGS2 = -Wl,-s -shared -pthread $(LDFLAGS)
OPTIMIZE = -O2
OPTIMIZE_FAST = -O3

ifdef WITH_PCSC_LIBRARY
  LDLIBS := -l$(WITH_PCSC_LIBRARY) $(LDLIBS)
else
  ifdef WITH_PCSC_PACKAGE
    PKG_CONFIG_RESULT := $(shell pkg-config $(WITH_PCSC_PACKAGE) --libs)
  else
    PKG_CONFIG_RESULT := $(shell pkg-config libpcsclite --libs)
  endif
  ifeq ($(PKG_CONFIG_RESULT),)
    $(error Failed)
  endif
  LDLIBS := $(PKG_CONFIG_RESULT) $(LDLIBS)
endif

B25Decoder.so: $(addsuffix .o,$(SRCS))
	$(CXX) $(LDFLAGS2) $^ $(LDLIBS) -o $@
BonProject__Multi2Decoder.cpp.o: BonProject/Multi2Decoder.cpp $(ALL_H)
	$(COMPILE.cpp) $(OPTIMIZE_FAST) $(OUTPUT_OPTION) $<
BonProject__%.cpp.o: BonProject/%.cpp $(ALL_H)
	$(COMPILE.cpp) $(OPTIMIZE) $(OUTPUT_OPTION) $<
%.cpp.o: %.cpp $(ALL_H)
	$(COMPILE.cpp) $(OPTIMIZE) $(OUTPUT_OPTION) $<
clean:
	$(RM) *.o B25Decoder.so

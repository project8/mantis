CC=g++
CFLAGS=-Wall -O2 -gstabs+
CSRC=$(wildcard ./src/*.cpp)

PXINCDIR=/usr/local/include
PXLIBDIR=/usr/local/lib
PXLIB=sig_px1500

MONINCDIR=/usr/local/include
MONLIBDIR=/usr/local/lib
MONLIB=monarch

PTHLIB=pthread
LIBDIRS=$(addprefix -L,$(PXLIBDIR) $(MONLIBDIR))
INCDIRS=$(addprefix -I,$(shell pwd)/include $(PXINCDIR) $(MONINCDIR))
LDFLAGS=$(addprefix -l,$(PXLIB) $(MONLIB))

BUILDDIR=build
OBJ=$(CSRC:%.cpp=%.o)
TGT=Mantis

$(TGT): $(OBJ)
	echo LD $@
	$(CC) -o $@ $(OBJ) $(LIBDIRS) $(LDFLAGS)

%.o: %.cpp $(BUILDIR)
	echo CXX $(basename $(notdir $@))
	$(CC) $(CFLAGS) $(INCDIRS) -c $< -o $@
	
test:
	$(CC) $(CFLAGS) $(INCDIRS) -c ./src/mantis_pci_test.cxx -o ./test
	chmod 744 ./test

install:
	cp $(TGT) /usr/local/bin/Mantis

clean: 
	@find . -maxdepth 2 -name "*.o" | xargs -I{} rm {}
	@find . -maxdepth 2 -name "*~" | xargs -I{} rm {}
	@find . -maxdepth 1 -name $(TGT) | xargs -I{} rm {}
	@echo cleaned.

$(BUILDIR): 
	@[ -d $@ ] || mkdir -p $@

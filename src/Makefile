CC = gcc
AR = ar cru
RANLIB = ranlib
MAJOR_VERSION = 0
MINOR_VERSION = 9
PATCH_LEVEL = 1
VERSION = $(MAJOR_VERSION).$(MINOR_VERSION).$(PATCH_LEVEL)
LIBDIR = /usr/lib
INCDIR = /usr/include
INCLUDE = -I../include
OUTPUT = libHVDI.so.$(VERSION)
LIBNAME = HVDI
STATIC = libHVDI.a
OPTFLAGS = -funroll-all-loops -ffast-math -fomit-frame-pointer -O2 -D_GNU_SOURCE
CFLAGS = -Wall -fPIC $(INCLUDE) $(OPTFLAGS)
LIBFLAGS = -shared -Wl,-soname,HVDI.so.$(MAJOR_VERSION).$(MINOR_VERSION) -rdynamic

HVDI_SOURCE = decpacket.c encpacket.c hvdi.c rate.c hcrypt.c

ADPCM_SOURCE = adpcm/adpcm.c

BLOWFISH_SOURCE = blowfish/bf_cbc.c blowfish/bf_cfb64.c blowfish/bf_ecb.c blowfish/bf_enc.c \
	blowfish/bf_ofb64.c blowfish/bf_skey.c

CELP_SOURCE = celp/autohf.c celp/celp_decode.c celp/celp_encode.c celp/csub.c \
	celp/gaincode.c celp/ham.c celp/lsp34.c celp/lsptopc.c celp/pctolsp2.c celp/pctolsp3.c \
	celp/pgain.c celp/pitchvq.c celp/polefilt.c celp/psearch.c celp/ptcode.c celp/zerofilt.c

GSM_SOURCE = gsm/gsm_state.c gsm/gsm_decode.c gsm/gsm_encode.c

LPC_SOURCE = lpc/lpcenc.c lpc/lpcdec.c

LPC10_SOURCE = lpc10/lpc10dec.c lpc10/lpc10enc.c

MD5_SOURCE = md5/md5.c

OPENLPC_SOURCE = openlpc/openlpc.c

ULAW_SOURCE = ulaw/u-law.c

OBJECTS = $(HVDI_SOURCE:.c=.o) \
	$(ADPCM_SOURCE:.c=.o) \
	$(BLOWFISH_SOURCE:.c=.o) \
	$(CELP_SOURCE:.c=.o) \
	$(GSM_SOURCE:.c=.o) \
	$(LPC_SOURCE:.c=.o) \
	$(LPC10_SOURCE:.c=.o) \
	$(MD5_SOURCE:.c=.o) \
	$(OPENLPC_SOURCE:.c=.o) \
	$(ULAW_SOURCE:.c=.o)


all: $(OBJECTS)
	$(CC) -o $(OUTPUT) $(OBJECTS) $(LIBFLAGS) $(CFLAGS)
	$(AR) $(STATIC) $(OBJECTS)
	$(RANLIB) $(STATIC)

include depend

install:
	cp $(OUTPUT) $(LIBDIR)
	cp $(STATIC) $(LIBDIR)
	chmod 755 $(LIBDIR)/$(OUTPUT)
	ln -s $(LIBDIR)/$(OUTPUT) lib$(LIBNAME).so.$(MAJOR_VERSION).$(MINOR_VERSION)
	ln -s $(LIBDIR)/$(OUTPUT) lib$(LIBNAME).so.$(MAJOR_VERSION)
	ln -s $(LIBDIR)/$(OUTPUT) lib$(LIBNAME).so
	cp hvdi.h $(INCDIR)/hvdi.h
	chmod 644 $(INCDIR)/hvdi.h
	@echo ""
	@echo "*************************************************"
	@echo "* Installed HawkVoiceDI.                             *"
	@echo "* Remember to run /sbin/ldconfig before using   *"
	@echo "* the library, you may also want to check that  *"
	@echo "* $(LIBDIR) is included in /etc/ld.so.conf      *"
	@echo "* You must be root to run ldconfig.             *"
	@echo "*************************************************"

uninstall:
	rm -f $(LIBDIR)/$(OUTPUT) $(LIBDIR)/lib$(LIBNAME).so.$(MAJOR_VERSION).$(MINOR_VERSION)
	rm -f $(LIBDIR)/lib$(LIBNAME).so.$(MAJOR_VERSION).$(MINOR_VERSION)
	rm -f $(LIBDIR)/lib$(LIBNAME).so.$(MAJOR_VERSION)
	rm -f $(LIBDIR)/lib$(LIBNAME).so
	rm -f $(LIBDIR)/$(STATIC)
	rm -f $(INCDIR)/hvdi.h

.PHONY : clean
clean:
	rm $(OBJECTS) $(OUTPUT) $(STATIC)

dep: $(HVDI_SOURCE) $(ADPCM_SOURCE) $(BLOWFISH_SOURCE) $(CELP_SOURCE) $(GSM_SOURCE) $(LPC_SOURCE) \
	$(LPC10_SOURCE) $(MD5_SOURCE) $(OPENLPC_SOURCE) $(ULAW_SOURCE)
	makedepend -fdepend -Y $(INCLUDE) $(HVDI_SOURCE) $(ADPCM_SOURCE) $(BLOWFISH_SOURCE) $(CELP_SOURCE) $(GSM_SOURCE) \
	$(LPC_SOURCE) $(LPC10_SOURCE) $(MD5_SOURCE) $(OPENLPC_SOURCE) $(ULAW_SOURCE)




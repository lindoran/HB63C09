CMOC = cmoc
SRINFO = srec_info
CAT = cat
FLEXFLAG = --flex
INTFLAG =


all: test.srec

int: INTFLAG = -i
int: test.srec

test.o: test.c fbios.h tinytypes.h
	$(CMOC) -c $(FLEXFLAG) $(INTFLAG) -o test.o test.c

flexfs.o: fbios.c fbios.h tinytypes.h
	$(CMOC) -c $(FLEXFLAG) $(INTFLAG) -o fbios.o fbios.c

test.srec: test.o flexfs.o
	$(CMOC) -o test.srec $(FLEXFLAG) $(INTFLAG) test.o fbios.o
	$(SRINFO) test.srec > test.srec.info
	$(CAT) test.srec.info




clean:
	rm -f *.o *.srec *.map *.link *.lst *.s *.info

.PHONY: all clean int
CC=gcc
LD=ld
CFLAGS =-fPIC -c
LFLAGS =-x --shared -lcrypto -ldl
OFILE=pam_zuul.so
SECDIR=/lib/security/

pam_zuul: 
	@$(CC) $(CFLAGS) $@.c -o $@.o
	@$(LD) -o $(OFILE) $(LFLAGS) $@.o
	@if [ -f $(OFILE) ]; then \
		echo "\nSuccessfully compiled, now run \"make install\" as root (or manually copy pam_zuul.so to /lib/security)\n"; \
	else \
		echo "Unable to compile. Please check error logs"; \
	fi
	
clean:
	@rm -f *.o
	@rm -f *.so
	@echo "Clean successful";

install:
	cp $(OFILE) $(SECDIR)
	@if [ -f $(SECDIR)$(OFILE) ]; then \
		echo "\nSuccessfully installed, now modify pam file accordingly\n"; \
	else \
		echo "Unable to install. Did you run as root?"; \
	fi



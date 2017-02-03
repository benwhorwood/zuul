CC=gcc
CFLAGS =-lcrypto -ldl
OFILE=aes_read_login_check_file
SECDIR=/lib/security/

aes_read_login_check_file: 
	@$(CC) $(CFLAGS) $@.c -o $@
	@if [ -f $(OFILE) ]; then \
		echo "\nSuccessfully compiled, now run \"$(OFILE)\"\n"; \
	else \
		echo "Unable to compile. Please check error logs"; \
	fi
	
clean:
	@rm -f *.o
	@rm -f $(OFILE)
	@echo "Clean successful";



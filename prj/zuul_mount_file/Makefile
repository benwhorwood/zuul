CC=gcc
CFLAGS=
OFILE=zuul_write_keyfile

main: 
	@$(CC) $(CFLAGS) $@.c -o $(OFILE)
	@if [ -f $(OFILE) ]; then \
		echo "\nSuccessfully compiled\n"; \
	else \
		echo "Unable to compile. Please check error logs"; \
	fi
	
clean:
	@rm -f *.o
	@rm -f $(OFILE)
	@echo "Clean successful";



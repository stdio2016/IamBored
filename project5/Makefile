HWNAME = banker
$(HWNAME): $(HWNAME).o
	$(CC) $+ -o $@ -pthread
$(HWNAME).o: $(HWNAME).c
clean:
	@rm -rf $(HWNAME) $(HWNAME).o

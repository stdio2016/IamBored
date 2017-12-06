objs = pokegym.o

pokegym: $(objs)
	$(CC) $+ -o $@ -pthread
pokegym.o: pokegym.c
clean:
	@rm -rf pokegym $(objs)

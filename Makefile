GCC = gcc

run: *.c
	$(GCC) $^ -o out
clean: out
	rm -rf $^


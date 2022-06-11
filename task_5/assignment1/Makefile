FILE_NAME=cpp_func.o 
# cpp_func.o
main.out: main.o c_func.o cpp_func.o asm_func.o
	g++ -o main.out main.o asm_func.o c_func.o cpp_func.o -m32 -g

c_func.o: c_func.c
	gcc -o c_func.o -m32 -c c_func.c -g

cpp_func.o: cpp_func.cpp
	g++ -o cpp_func.o -m32 -c cpp_func.cpp -g

main.o: main.cpp
	g++ -o main.o -m32 -c main.cpp -g

asm_func.o: asm_func.asm
	nasm -o asm_func.o -f elf32 asm_func.asm -g
	
run: main.out
	./main.out
clean:
	rm *.o

disassem:
	# make clean
	# make
	objdump -S $(FILE_NAME) 2>&1 | tee disassem.txt
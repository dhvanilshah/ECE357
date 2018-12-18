.PHONY: clean

testFIFO: testFIFO.o tas64.o sem.o fifo.o spin.o
	@echo "Building 'testTAS'..."
	@gcc -o testFIFO testFIFO.o tas64.o sem.o fifo.o spin.o

testTAS: testTAS.o tas64.o spin.o
	@echo "Building 'testTAS'..."
	@gcc -o testTAS testTAS.o tas64.o spin.o

testTAS.o: testTAS.c 
	@echo "Building 'testTAS.o'..."
	@gcc -c testTAS.c	

spin.o: spin.c spin.h 
	@echo "Building 'spin.o'..."
	@gcc -c spin.c

sem.o: sem.c sem.h 
	@echo "Building 'sem.o'..."
	@gcc -c sem.c	

fifo.o: fifo.c fifo.h 
	@echo "Building 'fifo.o'..."
	@gcc -c fifo.c	

testFIFO.o: testFIFO.c 
	@echo "Building 'testFIFO.o'..."
	@gcc -c testFIFO.c

tas64.o: tas64.S
	@echo "Building 'tas.o'..."
	@gcc -c tas64.S


clean:
	@echo "Removing all built files..."
	@rm -f *.o testTAS testFIFO
	@clear
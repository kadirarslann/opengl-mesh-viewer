
LDFLAGS=-lglut -lGLU -lGL -lGLEW

CC = g++
CFLAGS=-g -I/usr/include



example1: example1.o  InitShader.o 
	$(CC) $@.o InitShader.o $(LDFLAGS) -o $@

example2: example2.o  InitShader.o 
	$(CC) $@.o InitShader.o $(LDFLAGS) -o $@

hw2: hw2.o  InitShader.o 
	$(CC) $@.o InitShader.o $(LDFLAGS) -o $@


example3: example3.o  InitShader.o 
	$(CC) $@.o InitShader.o $(LDFLAGS) -o $@


%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -w


clean: 
	-rm -r example0 *.o core *.dSYM

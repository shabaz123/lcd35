all :
	gcc -c i2cfunc.c
	ar -rs libi2cfunc.a i2cfunc.o
	gcc -c lcd35-test.c
	gcc lcd35-test.o -L. -li2cfunc -lwiringPi -o lcd35-test
	chmod 755 lcd35-test




all : FileTest
	./FileTest

FileTest : FileTest.cpp ../src/FileHelperRK.cpp ../src/FileHelperRK.h ../src/FileHelperRK_AutomatedTest.h libwiringgcc
	gcc FileTest.cpp ../src/FileHelperRK.cpp UnitTestLib/libwiringgcc.a -std=c++17 -lc++ -IUnitTestLib -I../src -o FileTest -DUNITTEST

check : FileTest.cpp  ../src/FileHelperRK.cpp ../src/FileHelperRK.h libwiringgcc
	gcc FileTest.cpp ../src/FileHelperRK.cpp UnitTestLib/libwiringgcc.a -g -O0 -std=c++11 -lc++ -IUnitTestLib -I ../src -o FileTest && valgrind --leak-check=yes ./FileTest 

libwiringgcc :
	cd UnitTestLib && make libwiringgcc.a 		

clean :
	rm *.o FileTest || set status 0 
	cd UnitTestLib && make clean

.PHONY: libwiringgcc
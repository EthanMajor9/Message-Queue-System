# FINAL BINARY TARGET
./bin/dataReader : ./obj/dataReader.o
	cc ./obj/dataReader.o -o ./bin/dataReader

# DEPENDENCIES
./obj/dataReader.o : ./src/dataReader.c
	cc -c ./src/dataReader.c -o ./obj/dataReader.o

# OTHER TARGETS
all : ./bin/dataReader

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
	
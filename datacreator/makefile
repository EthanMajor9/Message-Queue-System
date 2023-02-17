# FINAL BINARY TARGET
./bin/dataCreator : ./obj/datacreator.o
	cc ./obj/datacreator.o -o ./bin/dataCreator

# DEPENDENCIES
./obj/datacreator.o : ./src/dataCreator.c
	cc -c ./src/dataCreator.c -o ./obj/datacreator.o

# OTHER TARGETS
all : ./bin/dataCreator

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
	
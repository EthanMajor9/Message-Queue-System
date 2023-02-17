# FINAL BINARY TARGET
./bin/datacorruptor : ./obj/datacorruptor.o
	cc ./obj/datacorruptor.o -o ./bin/datacorruptor

# DEPENDENCIES
./obj/datacorruptor.o : ./src/dataCorruptor.c
	cc -c ./src/dataCorruptor.c -o ./obj/datacorruptor.o

# OTHER TARGETS
all : ./bin/datacorruptor

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
	
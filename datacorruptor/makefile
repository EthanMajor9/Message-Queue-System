# FINAL BINARY TARGET
./bin/dataCorruptor : ./obj/dataCorruptor.o
	cc ./obj/dataCorruptor.o -o ./bin/dataCorruptor

# DEPENDENCIES
./obj/dataCorruptor.o : ./src/dataCorruptor.c ./inc/dataCorruptor.h
	cc -c ./src/dataCorruptor.c -o ./obj/dataCorruptor.o

# OTHER TARGETS
all : ./bin/dataCorruptor

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
	
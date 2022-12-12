CC = g++
INCPATH = -I /home/rhys/Documents/cpp/stocks/ecmd
CCFLAGS = -c -g -std=c++20
LIBS = -lcurl -lgumbo
OBJS = main.o Command.o

stocks : $(OBJS)
	$(CC) -o stocks $(OBJS) $(LIBS)

main.o : Command.o
	$(CC) $(CCFLAGS) main.cpp

Command.o : 
	$(CC) $(CCFLAGS) ecmd/Command.cpp

clean : 
	@rm *.o 2>/dev/null || true


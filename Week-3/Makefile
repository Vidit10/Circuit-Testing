CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = circuit_simulator
OBJS = main.o verilog_parser.o gate_logic.o circuit_node.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c verilog_parser.h gate_logic.h circuit_node.h
	$(CC) $(CFLAGS) -c main.c

verilog_parser.o: verilog_parser.c verilog_parser.h
	$(CC) $(CFLAGS) -c verilog_parser.c

gate_logic.o: gate_logic.c gate_logic.h
	$(CC) $(CFLAGS) -c gate_logic.c

circuit_node.o: circuit_node.c circuit_node.h gate_logic.h verilog_parser.h
	$(CC) $(CFLAGS) -c circuit_node.c

clean:
	rm -f $(OBJS) $(TARGET)

test: $(TARGET)
	./$(TARGET) c17.v

.PHONY: all clean test
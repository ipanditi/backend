#Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

# Executables
LOAD_BALANCER = advanced_load_balancer
SERVER1 = server1
SERVER2 = server2
SERVER3 = server3

# Source files
LOAD_BALANCER_SRC = advanced_load_balancer.cpp
SERVER1_SRC = server1.cpp BaseServer.cpp
SERVER2_SRC = server2.cpp BaseServer.cpp
SERVER3_SRC = server3.cpp BaseServer.cpp

# Object files
LOAD_BALANCER_OBJ = advanced_load_balancer.o
SERVER1_OBJ = server1.o BaseServer.o
SERVER2_OBJ = server2.o BaseServer.o
SERVER3_OBJ = server3.o BaseServer.o

#Targets
all: $(LOAD_BALANCER) $(SERVER1) $(SERVER2) $(SERVER3)

$(LOAD_BALANCER): $(LOAD_BALANCER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVER1): $(SERVER1_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVER2): $(SERVER2_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVER3): $(SERVER3_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
# Clean up
clean:
	rm -f $(LOAD_BALANCER) $(SERVER1) $(SERVER2) $(SERVER3) *.o network_traffic_log.csv

# Run all servers and load balancer, and then run traffic.sh
run: $(LOAD_BALANCER) $(SERVER1) $(SERVER2) $(SERVER3)
	@echo "Starting servers..."
	./$(SERVER1) &
	./$(SERVER2) &
	./$(SERVER3) &
	@sleep 2  # Give servers time to start
	@echo "Starting load balancer..."
	./$(LOAD_BALANCER) &
	@sleep 2  # Give load balancer time to start
	@echo "Running traffic simulation..."
	./traffic_testing.sh

#Kill
	@echo "Stopping servers and load balancer..."
	pkill -f $(SERVER1)
	pkill -f $(SERVER2)
	pkill -f $(SERVER3)
	pkill -f $(LOAD_BALANCER)

#rebuild
rebuild: clean all

.PHONY: all clean run rebuild


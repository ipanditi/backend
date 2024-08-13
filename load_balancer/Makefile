# Makefile for Load Balancer

CXX = g++
CXXFLAGS = -std=c++11 -pthread

SRCS = load_balancer.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = load_balancer

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS)

clean:
	rm -f $(OBJS) $(EXEC)


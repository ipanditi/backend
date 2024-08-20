FROM gcc:latest

# Set the working directory in the container
WORKDIR /app

# Install necessary packages
RUN apt-get update && \
    apt-get install -y cmake make g++ curl

# Copy all source files to the container
COPY . /app

# Compile the load balancer
#RUN g++ -o advanced_load_balancer advanced_load_balancer.cpp -lboost_system -pthread
RUN g++ -std=c++17 -pthread -o advanced_load_balancer advanced_load_balancer.cpp

# Make port 8080 available to the world outside this container
EXPOSE 8080
EXPOSE 8084
EXPOSE 8082
EXPOSE 8083
EXPOSE 8081 

# Define environment variable
ENV NAME World

# Run the load balancer when the container launches
CMD ["./advanced_load_balancer"]


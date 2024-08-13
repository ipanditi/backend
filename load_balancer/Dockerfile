# Use an official C++ base image
FROM gcc:latest

# Set the working directory in the container
WORKDIR /app

# Install necessary packages
RUN apt-get update && \
    apt-get install -y cmake make g++ curl

# Copy all source files to the container
COPY . /app

# Compile the code
RUN make rebuild

# Expose ports for the load balancer and servers
EXPOSE 8080
EXPOSE 8081
EXPOSE 8082
EXPOSE 8083

# Run the load balancer
CMD ["make", "run"]


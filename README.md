# Adaptive Load Balancer

This project implements an adaptive load balancer in C++ that dynamically adjusts its load balancing strategy based on real-time traffic conditions and server health. It supports three different algorithms — Round Robin, Least Connections, and Weighted Round Robin — and can handle multiple backend servers concurrently. This load balancer is designed to be scalable and can handle multiple clients simultaneously with multi-threading.

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
- [Load Balancing Algorithms](#load-balancing-algorithms)
  - [Round Robin](#round-robin)
  - [Least Connections](#least-connections)
  - [Weighted Round Robin](#weighted-round-robin)
- [Health Checks](#health-checks)
- [Logging](#logging)
- [Architecture](#architecture)
- [Usage](#usage)
- [Configuration](#configuration)
- [Contributing](#contributing)

## Features

- **Round Robin**: Cycles through the backend servers in order.
- **Least Connections**: Selects the server with the fewest active connections.
- **Weighted Round Robin**: Distributes traffic to backend servers based on their capacity (weight).
- **Health Checks**: Periodically monitors the health of backend servers and removes unhealthy servers from the pool.
- **Dynamic Algorithm Switching**: Switches between load balancing algorithms based on the current load and server health.
- **Multi-threaded**: Handles multiple client connections concurrently using threads.
- **Traffic Logging**: Logs traffic details, including timestamp, selected server, and algorithm used.
- **Connection Forwarding**: Relays traffic between clients and backend servers.
  
## Getting Started

### Prerequisites

To compile and run the load balancer, you will need:

- A C++17 compatible compiler (GCC or Clang recommended)
- POSIX compliant system (Linux, macOS)
- Networking libraries like `<sys/socket.h>`, `<arpa/inet.h>`, etc.

### Run 

1. Clone the repository:
   ```bash
   git clone https://github.com/ipanditi/backend.git
   cd load_balancer
   docker-compose up

## Load Balancing Algorithms
1. **Round Robin**
The load balancer selects servers in a circular order, distributing the traffic evenly across all available servers.

2. **Least Connections**
The load balancer dynamically chooses the server with the fewest active connections. This is useful when some servers are busier than others.

3. **Weighted Round Robin**
The load balancer selects servers based on their assigned weights. Servers with higher weights receive more requests compared to those with lower weights. This is helpful when backend servers have different capacities.

## Health Checks
The load balancer continuously monitors the health of backend servers by attempting to establish a TCP connection. Unhealthy servers are automatically removed from the pool and periodically rechecked. If no healthy servers are available, the load balancer waits for a server to recover.

## Logging
Traffic details, including selected backend server and load balancing algorithm, are logged into a CSV file (traffic_log.csv). The log captures:

- Timestamp
- Selected server
- Load balancing algorithm in use
This data helps analyze traffic distribution and detect any issues with load balancing behavior.

## Architecture
The load balancer is designed using a multi-threaded architecture that handles multiple client requests concurrently. Each client connection spawns a new thread to independently select a backend server, forward traffic, and manage the lifecycle of the connection.

- **Concurrency**: Multi-threading ensures multiple clients can be served at the same time.
- **Shared State**: A mutex (server_mutex) protects access to shared data such as the list of backend servers and active connections.
- **Non-blocking I/O**: The load balancer uses select() to handle traffic forwarding between the client and backend server.

## Flow of Operations
- The load balancer listens for client requests on a specified port (default: 8080).
- When a client connects, the load balancer selects a backend server based on the current algorithm.
- The load balancer performs a health check on the selected server. If the server is healthy, it forwards the client’s request.
- Traffic is relayed between the client and the backend server.
- Once the connection is closed, the load balancer decrements the active connection count for that server.
- Logs the request details and repeats for the next incoming client.

# To be implemented
1. Build a distributed file server using Go.
2. Video Transcoding using FFmPeg.
3. Real time streaming using NGINX and RTMP.
4. Way to go!


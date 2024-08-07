# Implemented
1. Built a simplistic load balancer that takes inputs from the clients hits the servers using round robin, gets the output, and sends it back to the client.
2. Added a feature that takes care of the case of failing servers(the most common backend fault), by implementing health checks whenever it is trying to access the server, if unhealthy, route it to the next healthy server in the queue.
3. Added a feature to replicate and multiply servers by implementing Inheritance of Base class Server (BaseServer), so that whenever all three servers are busy, a new server is added to the queue.

# To Be Implemented:
1. Robust implementation of addition of servers
2. When a server fails, it is unable to reconnect (once down, always down) so fix that (priority)
3. Make the server IPs dynamic
4. Implement different algos dynamically:
    1. Round Robin: Distributes requests sequentially across the server pool.
    2. Least Connections: Routes traffic to the server with the fewest active connections.
    3. IP Hash: Distributes requests based on the client's IP address.
    4. Random: Distributes requests randomly among the available servers.

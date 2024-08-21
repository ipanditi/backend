# Implemented
1. Built a simplistic load balancer that takes inputs from the clients hits the servers using round robin, gets the output, and sends it back to the client.
2. Added a feature that takes care of the case of failing servers(the most common backend fault), by implementing health checks whenever it is trying to access the server, if unhealthy, route it to the next healthy server in the queue.
3. Added a feature to replicate and multiply servers by implementing Inheritance of Base class Server (BaseServer), so that whenever all three servers are busy, a new server is added to the queue.
4. Implemented different algos dynamically:
    1. Round Robin: Distributes requests sequentially across the server pool.
    2. Least Connections: Routes traffic to the server with the fewest active connections.
    3. Weighted Round Robin: 
5. Traffic logs are dumped in a csv for further analysis.
6. Dockerized, now available as a Service (LaaS)

# To be implemented
1. Build a distributed file server using Go.
2. Video Transcoding using FFmPeg.
3. Real time streaming using NGINX and RTMP.
4. Way to go!


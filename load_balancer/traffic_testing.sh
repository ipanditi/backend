# Send initial requests to server 8081 to simulate it getting a load first
round_robin(){
   for i in {1..10}
   do
       curl -s http://127.0.0.1:8080 -o /dev/null &
   done
   wait
}

# Send a burst of requests to load balancer which will distribute them
# If Least Connections algorithm is triggered, it should prefer servers with fewer connections
least_connections(){
   for i in {1..5000}
   do
       curl -s http://127.0.0.1:8080 -o /dev/null &
   done
   wait
}
# Scenario 1: All servers up, testing Round Robin
echo "Scenario 1: Testing Round Robin with all servers healthy..."
round_robin
sleep 5

# Scenario 2: Simulate traffic spike, expecting switch to Least Connections
echo "Scenario 2: Simulating traffic spike for Least Connections..."
least_connections
sleep 5

# Take down one server (simulate by killing the process or blocking the port)
#echo "Simulating server 8081 failure..."
#sudo iptables -A INPUT -p tcp --dport 8081 -j DROP
#least_connections
#sleep 5

# Bring server back up
#echo "Bringing server 8081 back up..."
#sudo iptables -D INPUT -p tcp --dport 8081 -j DROP
#least_connections
#sleep 5

# Scenario 3: All servers up again, testing Weighted Round Robin
echo "Scenario 3: Testing Weighted Round Robin with all servers healthy..."
round_robin
sleep 5

echo "All scenarios completed. Check load balancer logs for results."


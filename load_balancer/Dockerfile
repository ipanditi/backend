FROM gcc:latest

#Install Boost Libraries
RUN apt-get update && apt-get install -y libboost-all-dev

COPY . /app
WORKDIR /app

RUN g++ -o load_balancer load_balancer.cpp -lbboost_system -lpthread

CMD ["./load_balancer", "8080", "backend1:8080", "backend2:8080"]


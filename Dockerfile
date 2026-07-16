FROM debian:stable-slim
RUN apt-get update
RUN apt-get install -y build-essential libc-dev gcc make
RUN apt-get install -y net-tools iproute2 iptables
RUN apt-get install -y valgrind
WORKDIR /app
COPY . .
RUN make re
CMD ["make"]

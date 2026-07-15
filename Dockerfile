FROM debian:stable-slim
RUN apt-get update
RUN apt-get install -y build-essential
RUN apt-get install -y libc-dev
RUN apt-get install -y gcc
RUN apt-get install -y make
RUN apt-get install -y net-tools
RUN apt-get install -y iproute2
RUN apt-get install -y valgrind
WORKDIR /app
COPY . .
RUN make re
CMD ["make"]

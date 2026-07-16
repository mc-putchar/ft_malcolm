## ft_malcolm

> [!WARNING]
> This project is for educational purposes only.  
> Do NOT spoof IPs that don't belong to you.  

Exploring the Address Resolution Protocol spoofing/poisoning  

> Yes, no, maybe  
> I don’t know  
> Can you repeat the question?  

### What?!

> [!NOTE]
> Allowed functions:  
> ◦ getuid, close, sigaction, signal, sleep  
> ◦ socket, setsockopt, sendto, recvfrom  
> ◦ inet_pton, inet_ntop, inet_addr, htons, ntohs  
> ◦ getaddrinfo, freeaddrinfo, strerror, gai_strerror, gethostbyname  
> ◦ getifaddrs, freeifaddrs, if_nametoindex  
> ◦ printf and its family

The program takes exactly 4 arguments, in this order:  
  1. Source IP
  2. Source MAC
  3. Target IP
  4. Target MAC

When started, the program waits for an ARP request sent on the broadcast by the target, requesting the source IP.  
The program sends a single ARP reply to the target and exits.  

### How?!

Setup VMs on the same network, or conveniently, use this example docker setup:

```bash
docker compose up -d
docker compose exec malcolm bash
```

Launch the program with:

```bash
./ft_malcolm 172.20.0.43 02:42:ac:14:00:42 172.20.0.41 02:42:ac:14:00:41
```

In another terminal:

```sh
docker compose exec reese watch -n 1 ip neigh show
```

And final one to shoot:

```sh
docker compose exec reese ping -c 1 dewey
```


### Why?!

Proof of concept for ARP cache poisoning.  
In theory, to escalate this into a real MITM attack, the program would need to continuously send ARP replies to keep the cache poisoned - and do so for (at least) two separate targets to intercept their communication.  
However, this is just an educational proof of concept, with no intention of such malicious use.


### Bonus

- [x] Support hostname resolution for IPv4 and IPv6
- [x] Verbose mode to print packet information
- [x] Decimal notation for IP addresses
- [x] Continuous mode to keep sending ARP replies

Argument options:
  - `-h` print help
  - `-v` verbose output
  - `-a` respond to any request for source IP
  - `-r` repeat sending ARP reply indefinitely

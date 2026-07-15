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
> ◦ inet_pton, inet_ntop, if_nametoindex, inet_addr, gethostbyname  
> ◦ getaddrinfo, freeaddrinfo, strerror, gai_strerror  
> ◦ getifaddrs, freeifaddrs, htons, ntohs  
> ◦ printf and its family

The program takes exactly 4 arguments, in this order:  
  1. Source IP
  2. Source MAC
  3. Target IP
  4. Target MAC

When started, the program waits for an ARP request sent on the broadcast by the target, requesting the source IP.  
The program sends a single ARP reply to the target and exits.

### How?!

```bash
docker compose up -d
docker compose exec malcolm bash
./ft_malcolm 172.20.0.69 02:42:ac:14:00:42 172.20.0.41 02:42:ac:14:00:41
```

In another terminal: 

```sh
docker compose exec reese watch -n 1 ip neigh show
```

And final one to shoot:

```sh
docker compose exec reese ping -c 1 172.20.0.69
```

We could also spoof the MAC assigned to `dewey`'s IP, but to avoid the cache being immediately overwritten by the real MAC address, we can disconnect `dewey` temporarily.  

```sh
docker network disconnect ft_malcolm_malnet dewey
```

### Why?!

Proof of concept for ARP cache poisoning.  
In theory, to escalate this into a real MITM attack, the program would need to continuously send ARP replies to keep the cache poisoned - and do so for (at least) two separate targets to intercept their communication.  
However, this is just an educational proof of concept, with no intention of such malicious use.

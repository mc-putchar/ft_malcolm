/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arp_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 03:17:54 by mcutura           #+#    #+#             */
/*   Updated: 2026/07/15 03:17:58 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ft_printf.h"
#include "libft.h"
#include "ft_malcolm.h"

struct s_ethhdr
{
	unsigned char	h_dest[ETH_ALEN];
	unsigned char	h_source[ETH_ALEN];
	__be16			h_proto;
} __attribute__((packed));

struct s_arphdr
{
	__be16			hw_type;
	__be16			proto_type;
	unsigned char	hw_len;
	unsigned char	proto_len;
	__be16			op;
	unsigned char	sender_mac[6];
	__be32			sender_ip;
	unsigned char	target_mac[6];
	__be32			target_ip;
} __attribute__((packed));

typedef struct s_arp_packet
{
	struct s_ethhdr	eth_hdr;
	struct s_arphdr	arp_hdr;
} __attribute__((packed))	t_arp_packet;

extern int	g_stop;

static int	is_ip_match(t_ip_addr *ip1, __be32 ip2)
{
	uint32_t	ip1_n;
	uint32_t	ip2_n;

	if (ip1->type != AF_INET)
	{
		ft_printf("IP type mismatch: expected AF_INET, got %d\n", ip1->type);
		return (0);
	}
	ip1_n = ip1->u_addr.ipv4.s_addr;
	ip1_n = (uint32_t)(ntohs((uint16_t)(ip1_n >> 16)) << 16) | \
		ntohs(ip1_n & 0xFFFF);
	ip2_n = (uint32_t)(ntohs((__be16)(ip2 >> 16)) << 16) | \
		ntohs((__be16)(ip2 & 0xFFFF));
	return (ip1_n == ip2_n);
}

void	arp_parse(struct s_arp_packet *arp_reply)
{
	char	buff[INET6_ADDRSTRLEN];

	ft_printf("ARP Packet:\n");
	ft_printf("  Sender IP: %s\n", \
		inet_ntop(AF_INET, &arp_reply->arp_hdr.sender_ip, buff, sizeof(buff)));
	ft_printf("  Sender MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", \
		arp_reply->arp_hdr.sender_mac[0], arp_reply->arp_hdr.sender_mac[1], \
		arp_reply->arp_hdr.sender_mac[2], arp_reply->arp_hdr.sender_mac[3], \
		arp_reply->arp_hdr.sender_mac[4], arp_reply->arp_hdr.sender_mac[5]);
	ft_printf("  Target IP: %s\n", \
		inet_ntop(AF_INET, &arp_reply->arp_hdr.target_ip, buff, sizeof(buff)));
	ft_printf("  Target MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", \
		arp_reply->arp_hdr.target_mac[0], arp_reply->arp_hdr.target_mac[1], \
		arp_reply->arp_hdr.target_mac[2], arp_reply->arp_hdr.target_mac[3], \
		arp_reply->arp_hdr.target_mac[4], arp_reply->arp_hdr.target_mac[5]);
}

void	build_arp_reply(struct s_arp_packet *arp_reply, t_device *source, \
	t_device *target)
{
	__be32 const	src_ip = source->ip.u_addr.ipv4.s_addr;
	__be32 const	tgt_ip = target->ip.u_addr.ipv4.s_addr;

	ft_bzero(arp_reply, sizeof(*arp_reply));
	ft_memcpy(arp_reply->eth_hdr.h_dest, &target->mac, ETH_ALEN);
	ft_memcpy(arp_reply->eth_hdr.h_source, &source->mac, ETH_ALEN);
	arp_reply->eth_hdr.h_proto = htons(ETH_P_ARP);
	arp_reply->arp_hdr.hw_type = htons(ARPHRD_ETHER);
	arp_reply->arp_hdr.proto_type = htons(ETH_P_IP);
	arp_reply->arp_hdr.hw_len = ETH_ALEN;
	arp_reply->arp_hdr.proto_len = sizeof(in_addr_t);
	arp_reply->arp_hdr.op = htons(ARPOP_REPLY);
	ft_memcpy(arp_reply->arp_hdr.sender_mac, &source->mac, ETH_ALEN);
	ft_memcpy(&arp_reply->arp_hdr.sender_ip, &src_ip, sizeof(src_ip));
	ft_memcpy(arp_reply->arp_hdr.target_mac, &target->mac, ETH_ALEN);
	ft_memcpy(&arp_reply->arp_hdr.target_ip, &tgt_ip, sizeof(tgt_ip));
	ft_printf("Payload ready. Running...\n");
}

int	arp_send(int sockfd, int ifindex, t_arp_packet *arp_reply, \
	t_mac_addr *src_mac)
{
	struct sockaddr_ll	sock_addr;
	ssize_t				sent;

	ft_printf("Sending reply to target:\n");
	arp_parse(arp_reply);
	ft_bzero(&sock_addr, sizeof(sock_addr));
	sock_addr.sll_family = ARPHRD_ETHER;
	sock_addr.sll_ifindex = ifindex;
	sock_addr.sll_halen = ETH_ALEN;
	ft_memcpy(sock_addr.sll_addr, src_mac, ETH_ALEN);
	sent = sendto(sockfd, arp_reply, sizeof(*arp_reply), 0, \
		(struct sockaddr *)&sock_addr, sizeof(sock_addr));
	if (sent < 0)
		return (throw_error(1, "Error: Failed to send ARP reply."));
	ft_printf("Sent ARP reply of size %u bytes\n", (unsigned int)sent);
	return (0);
}

int	arp_listen(int ifindex, t_device *iface, t_device *source, t_device *target)
{
	int const		sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	t_arp_packet	arp_reply;
	ssize_t			rb;
	unsigned char	buff[4096];

	if (sfd == -1 || setsockopt(sfd, SOL_SOCKET, SO_BINDTODEVICE, iface->name, \
		(socklen_t)ft_strlen(iface->name)) == -1)
		return (throw_error(1, "Error: Failed to create device bound socket."));
	build_arp_reply(&arp_reply, source, target);
	while (!g_stop)
	{
		rb = recvfrom(sfd, buff, sizeof(buff), 0, NULL, NULL);
		if (rb < 0)
			return (close(sfd), 1);
		if ((size_t)rb < sizeof(t_arp_packet))
			continue ;
		arp_parse((t_arp_packet *)buff);
		if (ntohs(((t_arp_packet *)buff)->eth_hdr.h_proto) == ETH_P_ARP \
		&& ntohs(((t_arp_packet *)buff)->arp_hdr.op) == ARPOP_REQUEST \
		&& is_ip_match(&target->ip, ((t_arp_packet *)buff)->arp_hdr.sender_ip) \
		&& is_ip_match(&source->ip, ((t_arp_packet *)buff)->arp_hdr.target_ip) \
		&& !arp_send(sfd, ifindex, &arp_reply, &source->mac))
			return (close(sfd), 0);
	}
	return (close(sfd), 0);
}

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
#include <net/if_arp.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

#include "libft.h"
#include "ft_malcolm.h"

typedef struct s_arp_packet
{
	struct s_ethhdr	eth_hdr;
	struct s_arphdr	arp_hdr;
} __attribute__((packed))	t_arp_packet;

extern sig_atomic_t	g_stop;

static void	arp_print(struct s_arp_packet *arp)
{
	char	buff[INET6_ADDRSTRLEN];

	ft_printf("ARP Packet:");
	if (ntohs(arp->arp_hdr.op) == 1)
		ft_printf("  REQUEST\n");
	else if (ntohs(arp->arp_hdr.op) == 2)
		ft_printf("  REPLY\n");
	ft_printf("  Sender IP:  %s\n", \
		inet_ntop(AF_INET, &arp->arp_hdr.sender_ip, buff, sizeof(buff)));
	ft_printf("  Sender MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", \
		arp->arp_hdr.sender_mac[0], arp->arp_hdr.sender_mac[1], \
		arp->arp_hdr.sender_mac[2], arp->arp_hdr.sender_mac[3], \
		arp->arp_hdr.sender_mac[4], arp->arp_hdr.sender_mac[5]);
	ft_printf("  Target IP:  %s\n", \
		inet_ntop(AF_INET, &arp->arp_hdr.target_ip, buff, sizeof(buff)));
	ft_printf("  Target MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", \
		arp->arp_hdr.target_mac[0], arp->arp_hdr.target_mac[1], \
		arp->arp_hdr.target_mac[2], arp->arp_hdr.target_mac[3], \
		arp->arp_hdr.target_mac[4], arp->arp_hdr.target_mac[5]);
}

static void	build_arp_reply(struct s_arp_packet *arp_reply, t_device *source, \
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
	arp_reply->arp_hdr.op = htons(ARPOP_REQUEST);
	ft_memcpy(arp_reply->arp_hdr.sender_mac, &source->mac, ETH_ALEN);
	ft_memcpy(&arp_reply->arp_hdr.sender_ip, &src_ip, sizeof(src_ip));
	ft_memcpy(arp_reply->arp_hdr.target_mac, &target->mac, ETH_ALEN);
	ft_memcpy(&arp_reply->arp_hdr.target_ip, &tgt_ip, sizeof(tgt_ip));
}

static int	arp_send(uint32_t flags, t_hw_info info, t_device *source, \
	t_device *target)
{
	ssize_t				sent;
	struct sockaddr_ll	sock_addr;
	t_arp_packet		arp_reply;

	build_arp_reply(&arp_reply, source, target);
	while (!g_stop)
	{
		ft_printf("Sending reply to target\n");
		if (flags & (1 << FL_VERBOSE))
			arp_print(&arp_reply);
		ft_bzero(&sock_addr, sizeof(sock_addr));
		sock_addr.sll_family = ARPHRD_ETHER;
		sock_addr.sll_ifindex = info.if_index;
		sock_addr.sll_halen = ETH_ALEN;
		ft_memcpy(sock_addr.sll_addr, &source->mac, ETH_ALEN);
		sent = sendto(info.socket, &arp_reply, sizeof(arp_reply), 0, \
			(struct sockaddr *)&sock_addr, sizeof(sock_addr));
		if (sent < 0)
			return (throw_error(1, "Error: Failed to send ARP reply."));
		ft_printf("Sent ARP reply of size %u bytes\n", (unsigned int)sent);
		if ((flags ^ (1 << FL_REPEAT)) & (1 << FL_REPEAT))
			break ;
		sleep(1);
	}
	return (0);
}

static int	aim(t_device *target, t_arp_packet *arp)
{
	target->ip.type = AF_INET;
	target->ip.u_addr.ipv4.s_addr = arp->arp_hdr.sender_ip;
	ft_memcpy(target->mac.addr, arp->arp_hdr.sender_mac, ETH_ALEN);
	return (1);
}

int	arp_listen(uint32_t flags, t_device *iface, t_device *source, \
	t_device *target)
{
	int const		sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	ssize_t			rb;
	unsigned char	buff[4096];

	if (sfd == -1 || setsockopt(sfd, SOL_SOCKET, SO_BINDTODEVICE, iface->name, \
		(socklen_t)ft_strlen(iface->name)) == -1)
		return (throw_error(1, "Error: Failed to create device bound socket."));
	if (flags & (1 << FL_ACTIVE))
		return (arp_send(flags, (t_hw_info){sfd, iface->idx}, source, target));
	while (!g_stop)
	{
		rb = recvfrom(sfd, buff, sizeof(buff), 0, NULL, NULL);
		if (rb < 0 || (size_t)rb < sizeof(t_arp_packet))
			return (close(sfd), 1);
		if (flags & (1 << FL_VERBOSE))
			arp_print((t_arp_packet *)buff);
		if (ntohs(((t_arp_packet *)buff)->eth_hdr.h_proto) == ETH_P_ARP \
		&& ntohs(((t_arp_packet *)buff)->arp_hdr.op) == ARPOP_REQUEST \
		&& is_ip_match(&source->ip, ((t_arp_packet *)buff)->arp_hdr.target_ip) \
		&& (!(flags & (1 << FL_TGTANY)) || aim(target, (t_arp_packet *)buff)) \
		&& is_ip_match(&target->ip, ((t_arp_packet *)buff)->arp_hdr.sender_ip) \
		&& !arp_send(flags, (t_hw_info){sfd, iface->idx}, source, target))
			return (close(sfd), 0);
	}
	return (close(sfd), 0);
}

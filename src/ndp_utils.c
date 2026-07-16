/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ndp_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 15:31:51 by mcutura           #+#    #+#             */
/*   Updated: 2026/07/16 15:31:53 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <linux/if_ether.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>

#include "ft_malcolm.h"
#include "ft_printf.h"
#include "libft.h"

extern sig_atomic_t	g_stop;

typedef struct s_na_payload
{
	struct nd_neighbor_advert	na_hdr;
	struct nd_opt_hdr			opt;
	unsigned char				mac[6];
} __attribute__((packed))	t_na_payload;

static void	print_ndp_packet(unsigned char *buff, ssize_t len)
{
	struct icmp6_hdr *const		hdr = (struct icmp6_hdr *)buff;
	struct nd_opt_hdr			*opt;
	char						ip_str[INET6_ADDRSTRLEN];
	unsigned char				*mac;

	ft_printf("NDP Packet:  ");
	if (hdr->icmp6_type == ND_NEIGHBOR_SOLICIT)
		ft_printf("NEIGH SOLICIT:  Who has %s\n", inet_ntop(AF_INET6, \
			&((struct nd_neighbor_solicit *)buff)->nd_ns_target, \
			ip_str, INET6_ADDRSTRLEN));
	else if (hdr->icmp6_type == ND_NEIGHBOR_ADVERT)
	{
		ft_printf("NEIGH ADVERT:  Target IP: %s\n", inet_ntop(AF_INET6, \
			&((struct nd_neighbor_advert *)buff)->nd_na_target, ip_str, \
			INET6_ADDRSTRLEN));
		if ((size_t)len <= sizeof(struct nd_neighbor_advert))
			return ;
		opt = (struct nd_opt_hdr *)(buff + sizeof(struct nd_neighbor_advert));
		if (opt->nd_opt_type == ND_OPT_TARGET_LINKADDR)
		{
			mac = (unsigned char *)opt + 2;
			ft_printf("  Link Address: %02x:%02x:%02x:%02x:%02x:%02x\n", \
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}
}

static int	join_sn_multicast(uint32_t flags, int sfd, t_device *iface, \
	struct in6_addr *target_ip)
{
	static unsigned char const	prefix[13] = {
		0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0xff
	};
	static int const			hoplimit = 255;
	int							optname;
	struct ipv6_mreq			mreq;
	struct in6_addr				mcast;

	optname = IPV6_UNICAST_HOPS;
	if (flags & (1 << FL_ACTIVE))
		optname = IPV6_MULTICAST_HOPS;
	if (sfd < 0 || setsockopt(sfd, SOL_SOCKET, SO_BINDTODEVICE, iface->name, \
		(socklen_t)ft_strlen(iface->name)) || setsockopt(sfd, IPPROTO_IPV6, \
		optname, &hoplimit, sizeof(hoplimit)))
		return (throw_error(1, "Error: Failed to create device bound socket."));
	ft_bzero(&mcast, sizeof(mcast));
	ft_memcpy(mcast.s6_addr, prefix, 13);
	ft_memcpy(mcast.s6_addr + 13, target_ip->s6_addr + 13, 3);
	ft_bzero(&mreq, sizeof(mreq));
	mreq.ipv6mr_interface = (unsigned int)iface->idx;
	ft_memcpy(&mreq.ipv6mr_multiaddr, &mcast, sizeof(mcast));
	if (setsockopt(sfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)))
		return (throw_error(1, "Error: Failed to join target multicast group"));
	return (0);
}

static void	build_na_packet(uint32_t flags, t_na_payload *na_packet, \
	t_device *source)
{
	ft_bzero(na_packet, sizeof(t_na_payload));
	na_packet->na_hdr.nd_na_hdr.icmp6_type = ND_NEIGHBOR_ADVERT;
	na_packet->na_hdr.nd_na_hdr.icmp6_code = 0;
	na_packet->na_hdr.nd_na_hdr.icmp6_dataun.icmp6_un_data32[0] = \
		ND_NA_FLAG_SOLICITED | ND_NA_FLAG_OVERRIDE;
	if (flags & (1 << FL_ACTIVE))
		na_packet->na_hdr.nd_na_hdr.icmp6_dataun.icmp6_un_data32[0] = \
			ND_NA_FLAG_OVERRIDE;
	na_packet->opt.nd_opt_type = ND_OPT_TARGET_LINKADDR;
	na_packet->opt.nd_opt_len = 1;
	ft_memcpy(na_packet->mac, source->mac.addr, ETH_ALEN);
	ft_memcpy(&na_packet->na_hdr.nd_na_target, &source->ip.u_addr.ipv6, \
		sizeof(struct in6_addr));
}

static int	send_na_reply(uint32_t flags, t_hw_info info, t_device *source, \
	struct in6_addr *tgt_ip)
{
	t_na_payload					na_packet;
	struct sockaddr_in6				dest;
	ssize_t							sent;

	build_na_packet(flags, &na_packet, source);
	ft_bzero(&dest, sizeof(dest));
	dest.sin6_family = AF_INET6;
	dest.sin6_scope_id = (uint32_t)info.if_index;
	if (!tgt_ip || !ft_memcpy(&dest.sin6_addr, tgt_ip, sizeof(struct in6_addr)))
		inet_pton(AF_INET6, ALLNODES_MULTICAST, &dest.sin6_addr);
	while (!g_stop)
	{
		if (flags & (1 << FL_VERBOSE))
			print_ndp_packet((unsigned char *)&na_packet, sizeof(na_packet));
		sent = sendto(info.socket, &na_packet, sizeof(na_packet), 0, \
			(struct sockaddr *)&dest, sizeof(dest));
		if (sent < 0)
			return (throw_error(1, "Error: Failed to send NA."));
		ft_printf("Sent NA reply of size %u bytes\n", (unsigned int)sent);
		if ((flags ^ (1 << FL_REPEAT)) & (1 << FL_REPEAT))
			break ;
		sleep(1);
	}
	return (1);
}

int	ndp_listen(uint32_t flags, t_device *iface, t_device *source, \
	t_device *target)
{
	int const		sfd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	ssize_t			rb;
	unsigned char	buff[4096];

	if (join_sn_multicast(flags, sfd, iface, &target->ip.u_addr.ipv6) \
	|| (flags & (1 << FL_ACTIVE) && send_na_reply(flags, \
		(t_hw_info){sfd, iface->idx}, source, NULL)))
		return (close(sfd), 1);
	while (!g_stop)
	{
		rb = recvfrom(sfd, buff, sizeof(buff), 0, NULL, NULL);
		if (rb < 0)
			return (close(sfd), 1);
		if ((size_t)rb < sizeof(struct icmp6_hdr))
			continue ;
		if (flags & (1 << FL_VERBOSE))
			print_ndp_packet(buff, rb);
		if (((struct icmp6_hdr *)buff)->icmp6_type == ND_NEIGHBOR_SOLICIT \
		&& !ft_memcmp(&((struct nd_neighbor_solicit *)buff)->nd_ns_target, \
			&source->ip.u_addr.ipv6, sizeof(struct in6_addr)) \
		&& send_na_reply(flags, (t_hw_info){sfd, iface->idx}, source, \
			&target->ip.u_addr.ipv6))
			return (close(sfd), 1);
	}
	return (close(sfd), 0);
}

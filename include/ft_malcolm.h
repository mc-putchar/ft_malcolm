/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_malcolm.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 03:17:40 by mcutura           #+#    #+#             */
/*   Updated: 2026/07/15 03:17:44 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_MALCOLM_H
# define FT_MALCOLM_H

# include <stdint.h>
# include <arpa/inet.h>
# include <netinet/ether.h>
# include "net/if.h"

# define FLAGS			"hrvat"
# define FL_HELP		0
# define FL_REPEAT		1
# define FL_VERBOSE		2
# define FL_ACTIVE		3
# define FL_TGTANY		4

# define ALLNODES_MULTICAST "ff02::1"

typedef struct s_ip_addr
{
	int		type;
	char	str[INET6_ADDRSTRLEN];
	union {
		struct in_addr	ipv4;
		struct in6_addr	ipv6;
	}	u_addr;
}	t_ip_addr;

typedef struct s_mac_addr
{
	unsigned char	addr[6];
}	t_mac_addr;

typedef struct s_device
{
	char		name[IFNAMSIZ];
	t_mac_addr	mac;
	t_ip_addr	ip;
	int			idx;
}	t_device;

typedef struct s_hw_info
{
	int	socket;
	int	if_index;
}	t_hw_info;

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

int		set_signal_handlers(void);
int		parse_ip_addr(char *src, t_ip_addr *addr);
int		resolve_hostname(char *hostname, t_ip_addr *ip);
int		is_ip_match(t_ip_addr *ip1, uint32_t ip2);
int		parse_mac_addr(char *str, t_mac_addr *mac_addr);
ssize_t	get_available_interface(t_device *interface, int src_type);
void	print_interface_info(t_device *interface);
int		arp_listen(uint32_t flags, t_device *iface, \
	t_device *source, t_device *target);
int		ndp_listen(uint32_t flags, t_device *iface, \
	t_device *source, t_device *target);

#endif //FT_MALCOLM_H

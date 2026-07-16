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

# include <arpa/inet.h>
# include <stdint.h>
# include "net/if.h"

# define FLAGS			"hrva"
# define FL_HELP		0
# define FL_REPEAT		1
# define FL_VERBOSE		2
# define FL_ANYTGT		3

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

int		set_signal_handlers(void);
int		parse_ip_addr(char *src, t_ip_addr *addr);
int		resolve_hostname(char *hostname, t_ip_addr *ip);
int		is_ip_match(t_ip_addr *ip1, uint32_t ip2);
int		parse_mac_addr(char *str, t_mac_addr *mac_addr);
ssize_t	get_available_interface(t_device *interface);
void	print_interface_info(t_device *interface);
int		arp_listen(uint32_t flags, t_device *iface, \
	t_device *source, t_device *target);

#endif //FT_MALCOLM_H

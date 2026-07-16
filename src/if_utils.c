/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   if_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 03:18:02 by mcutura           #+#    #+#             */
/*   Updated: 2026/07/15 03:18:03 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include "net/if.h"

#include "libft.h"
#include "ft_malcolm.h"

static void	extract_info(int fam, struct ifaddrs *i, t_device *ifa)
{
	struct sockaddr_in	*sin;
	struct sockaddr_in6	*sin6;
	struct sockaddr_ll	*sll;

	if (fam == AF_INET)
	{
		sin = (struct sockaddr_in *)i->ifa_addr;
		ft_memcpy(&ifa->ip.u_addr, &sin->sin_addr, sizeof(struct in_addr));
		ifa->ip.type = AF_INET;
		inet_ntop(AF_INET, &sin->sin_addr, ifa->ip.str, INET_ADDRSTRLEN);
	}
	else if (fam == AF_INET6)
	{
		sin6 = (struct sockaddr_in6 *)i->ifa_addr;
		ft_memcpy(&ifa->ip.u_addr, &sin6->sin6_addr, sizeof(struct in6_addr));
		ifa->ip.type = AF_INET6;
		inet_ntop(AF_INET6, &sin6->sin6_addr, ifa->ip.str, INET6_ADDRSTRLEN);
	}
	else if (fam == AF_PACKET)
	{
		sll = (struct sockaddr_ll *)i->ifa_addr;
		ft_memcpy(ifa->mac.addr, sll->sll_addr, sizeof(ifa->mac.addr));
	}
	ft_strlcpy(ifa->name, i->ifa_name, IFNAMSIZ);
}

ssize_t	get_available_interface(t_device *interface)
{
	struct ifaddrs	*ifap;
	struct ifaddrs	*i;

	if (getifaddrs(&ifap))
		return (throw_error(-1, "Error: getifaddrs failed."));
	i = ifap;
	interface->idx = -1;
	while (i)
	{
		if (i->ifa_addr && i->ifa_flags & (IFF_UP | IFF_BROADCAST) \
		&& !(i->ifa_flags & (IFF_LOOPBACK | IFF_NOARP)))
		{
			if (interface->idx < 0)
			{
				interface->idx = (int)if_nametoindex(i->ifa_name);
				ft_strlcpy(interface->name, i->ifa_name, IFNAMSIZ);
			}
			if (interface->idx == (int)if_nametoindex(i->ifa_name))
				extract_info(i->ifa_addr->sa_family, i, interface);
		}
		i = i->ifa_next;
	}
	freeifaddrs(ifap);
	return (interface->idx);
}

void	print_interface_info(t_device *interface)
{
	ft_printf("%s\n", interface->name);
	ft_printf("  IP:  %s\n", interface->ip.str);
	ft_printf("  MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
		interface->mac.addr[0], interface->mac.addr[1], interface->mac.addr[2],
		interface->mac.addr[3], interface->mac.addr[4], interface->mac.addr[5]);
}

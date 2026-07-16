/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ip_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 03:18:07 by mcutura           #+#    #+#             */
/*   Updated: 2026/07/15 03:18:12 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

#include "libft.h"
#include "ft_malcolm.h"

static int	derive_type(char *src)
{
	if (ft_strchr(src, '.'))
		return (AF_INET);
	if (ft_strchr(src, ':'))
		return (AF_INET6);
	return (-1);
}

int	parse_ip_addr(char *src, t_ip_addr *ip)
{
	int const	type = derive_type(src);

	if (type == AF_INET)
	{
		ip->type = AF_INET;
		if (inet_pton(type, src, &ip->u_addr.ipv4) != 1 \
		|| !inet_ntop(type, &ip->u_addr.ipv4, ip->str, INET6_ADDRSTRLEN))
			return (resolve_hostname(src, ip));
	}
	else if (type == AF_INET6)
	{
		ip->type = AF_INET6;
		if (inet_pton(type, src, &ip->u_addr.ipv6) != 1 \
		|| !inet_ntop(type, &ip->u_addr.ipv6, ip->str, INET6_ADDRSTRLEN))
			return (resolve_hostname(src, ip));
	}
	else
		return (resolve_hostname(src, ip));
	return (0);
}

static int	extract_ip(t_ip_addr *ip, struct sockaddr *addr)
{
	ip->type = addr->sa_family;
	if (addr->sa_family == AF_INET)
	{
		ft_memcpy(&ip->u_addr.ipv4.s_addr, \
			&((struct sockaddr_in *)addr)->sin_addr, \
			sizeof(ip->u_addr.ipv4.s_addr));
		inet_ntop(AF_INET, &ip->u_addr.ipv4, ip->str, INET_ADDRSTRLEN);
	}
	else if (addr->sa_family == AF_INET6)
	{
		ft_memcpy(&ip->u_addr.ipv6.s6_addr, \
			&((struct sockaddr_in6 *)addr)->sin6_addr, \
			sizeof(ip->u_addr.ipv6.s6_addr));
		inet_ntop(AF_INET6, &ip->u_addr.ipv6, ip->str, INET6_ADDRSTRLEN);
	}
	else
		return (0);
	return (1);
}

int	resolve_hostname(char *hostname, t_ip_addr *ip)
{
	struct addrinfo	*res;
	struct addrinfo	*ai;

	if (getaddrinfo(hostname, NULL, NULL, &res))
		return (1);
	ai = res;
	while (ai)
	{
		if (extract_ip(ip, ai->ai_addr))
			break ;
		ai = ai->ai_next;
	}
	freeaddrinfo(res);
	return (ai == NULL);
}

int	is_ip_match(t_ip_addr *ip1, uint32_t ip2)
{
	uint32_t	ip1_n;
	uint32_t	ip2_n;

	if (ip1->type == AF_INET)
	{
		ip1_n = ip1->u_addr.ipv4.s_addr;
		ip1_n = (uint32_t)(ntohs((uint16_t)(ip1_n >> 16)) << 16) | \
			ntohs(ip1_n & 0xFFFF);
		ip2_n = (uint32_t)(ntohs((uint16_t)(ip2 >> 16)) << 16) | \
			ntohs((uint16_t)(ip2 & 0xFFFF));
		return (ip1_n == ip2_n);
	}
	ft_printf("IP type mismatch: expected AF_INET, got %d\n", ip1->type);
	return (0);
}

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

#include <stdlib.h>
#include <arpa/inet.h>

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
		if (inet_pton(type, src, &ip->u_addr.ipv4) != 1 \
		|| !inet_ntop(type, &ip->u_addr.ipv4, ip->str, INET6_ADDRSTRLEN))
			return (1);
	}
	else if (type == AF_INET6)
	{
		if (inet_pton(type, src, &ip->u_addr.ipv6) != 1 \
		|| !inet_ntop(type, &ip->u_addr.ipv6, ip->str, INET6_ADDRSTRLEN))
			return (1);
	}
	else
		return (1);
	ip->type = type;
	return (0);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mac_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 03:18:24 by mcutura           #+#    #+#             */
/*   Updated: 2026/07/15 03:18:28 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stddef.h>

#include "libft.h"
#include "ft_malcolm.h"

#define MAC_DELIM	":-."

static int	xtoi(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (-1);
}

int	parse_mac_addr(char *str, t_mac_addr *mac_addr)
{
	size_t	i;
	int		val;

	if (ft_strlen(str) != 17)
		return (1);
	ft_bzero(mac_addr->addr, sizeof(mac_addr->addr));
	i = 0;
	while (i++ < 17)
	{
		if (i % 3 == 0)
		{
			if (!ft_strchr(MAC_DELIM, str[i - 1]))
				return (1);
			continue ;
		}
		val = xtoi(str[i - 1]);
		if (val < 0)
			return (1);
		mac_addr->addr[i / 3] <<= 4;
		mac_addr->addr[i / 3] |= (unsigned char)val;
	}
	return (0);
}

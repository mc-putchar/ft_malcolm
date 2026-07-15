/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 03:18:33 by mcutura           #+#    #+#             */
/*   Updated: 2026/07/15 03:18:34 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>

#include "libft.h"
#include "ft_malcolm.h"

#define USAGE \
"Usage: %s <SOURCE_IP> <SOURCE_MAC> <TARGET_IP> <TARGET_MAC>\n"

static int	parse_args(char **av, t_device *source, t_device *target)
{
	if (parse_ip_addr(av[1], &source->ip) \
	|| parse_mac_addr(av[2], &source->mac) \
	|| parse_ip_addr(av[3], &target->ip) \
	|| parse_mac_addr(av[4], &target->mac))
		return (1);
	return (0);
}

int	main(int ac, char **av)
{
	ssize_t		if_idx;
	t_device	interface;
	t_device	source;
	t_device	target;

	if (getuid() != 0)
	{
		if (ac != 5)
			ft_dprintf(STDERR_FILENO, USAGE, av[0]);
		return (throw_error(1, "Error: This program must run as root."));
	}
	if (ac != 5)
	{
		ft_dprintf(STDERR_FILENO, USAGE, av[0]);
		return (1);
	}
	if (parse_args(av, &source, &target))
		return (throw_error(1, "Error: Invalid arguments."));
	if (set_signal_handlers())
		return (throw_error(1, "Error: Failed to set signal handlers."));
	if_idx = get_available_interface(&interface);
	if (if_idx == -1)
		return (throw_error(1, "Error: No available network interface found."));
	print_interface_info(&interface);
	return (arp_listen((int)if_idx, &interface, &source, &target));
}

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

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include "ft_printf.h"
#include "libft.h"
#include "ft_malcolm.h"

#define USAGE \
"Usage: \n \
    %s [OPTIONS] <SOURCE_IP> <SOURCE_MAC> <TARGET_IP> <TARGET_MAC>\n \
\nOptions:\n \
    -h  Display this help message\n \
    -r  Repeat sending ARP reply indefinitely\n \
    -v  Verbose output - print packet information\n \
    -a  Active mode - send unsolicited reply\n \
    -t  Reply to any target request for source IP\n"

static int	parse_flags(uint32_t *flags, char *str)
{
	char	*flag;
	int		i;

	i = 0;
	while (str[++i])
	{
		flag = ft_strchr(FLAGS, str[i]);
		if (!flag)
			return (1);
		*flags |= 1 << (flag - FLAGS);
	}
	return (0);
}

static ssize_t	parse_args(int ac, char **av, t_device *source, \
	t_device *target)
{
	uint32_t	flags;
	int			i;
	int			j;

	*source = (t_device){0};
	*target = (t_device){0};
	flags = 0;
	i = 0;
	j = 0;
	while (++i < ac)
	{
		if (av[i][0] != '-')
		{
			if ((j < 2 && ((j & 1 && parse_mac_addr(av[i], &source->mac)) \
				|| (!(j & 1) && parse_ip_addr(av[i], &source->ip)))) \
			|| (j > 1 && ((j & 1 && parse_mac_addr(av[i], &target->mac)) \
				|| (!(j & 1) && parse_ip_addr(av[i], &target->ip)))))
				return (throw_error(-1, "Error: Invalid arguments."));
			j++;
		}
		else if (parse_flags(&flags, av[i]))
			return (throw_error(-1, "Error: Invalid flags."));
	}
	return (flags);
}

int	main(int ac, char **av)
{
	ssize_t		flags;
	t_device	interface;
	t_device	source;
	t_device	target;

	if (getuid())
	{
		if (ac < 5)
			ft_dprintf(STDERR_FILENO, USAGE, av[0]);
		return (throw_error(1, "\nError: This program must run as root."));
	}
	flags = parse_args(ac, av, &source, &target);
	if (flags < 0 || flags & (1 << FL_HELP) || ac < 5)
	{
		ft_dprintf(STDERR_FILENO, USAGE, av[0]);
		return (flags < 0 || ((flags & (1 << FL_HELP)) != (1 << FL_HELP)));
	}
	if (set_signal_handlers())
		return (throw_error(1, "Error: Failed to set signal handlers."));
	if (get_available_interface(&interface, source.ip.type) < 0)
		return (throw_error(1, "Error: No available network interface found."));
	print_interface_info(&interface);
	if (source.ip.type == AF_INET && target.ip.type == AF_INET)
		return (arp_listen((uint32_t)flags, &interface, &source, &target));
	return (ndp_listen((uint32_t)flags, &interface, &source, &target));
}

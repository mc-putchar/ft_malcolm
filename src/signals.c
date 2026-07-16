/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 03:18:39 by mcutura           #+#    #+#             */
/*   Updated: 2026/07/15 03:18:40 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <stddef.h>

sig_atomic_t	g_stop = 0;

static void	handle_signal(int sig)
{
	(void)sig;
	g_stop = 1;
}

int	set_signal_handlers(void)
{
	struct sigaction	sa;

	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1 \
	|| sigaction(SIGTERM, &sa, NULL) == -1)
		return (1);
	return (0);
}

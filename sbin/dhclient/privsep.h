/*	$OpenBSD: privsep.h,v 1.64 2020/05/19 17:59:47 krw Exp $ */

/*
 * Copyright (c) 2004 Henning Brauer <henning@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE, ABUSE OR PERFORMANCE OF THIS SOFTWARE.
 */

enum imsg_code {
	IMSG_NONE,
	IMSG_REVOKE,
	IMSG_WRITE_RESOLV_CONF,
	IMSG_PROPOSE,
	IMSG_TELL_UNWIND
};

#define	RTLEN	128

struct proposal {
	uint8_t		rtstatic[RTLEN];
	uint8_t		rtsearch[RTLEN];
	uint8_t		rtdns[RTLEN];
	struct in_addr	ifa;
	struct in_addr	netmask;
	unsigned int	rtstatic_len;
	unsigned int	rtsearch_len;
	unsigned int	rtdns_len;
	int		mtu;
};

struct unwind_info {
	in_addr_t	ns[MAXNS];
	unsigned int	count;
};

void	dispatch_imsg(char *, int, int, int, struct imsgbuf *);

void	priv_write_resolv_conf(int, int, int, char *, int *);
void	priv_propose(char *, int, struct proposal *, char **, int, int, int);

void	priv_revoke_proposal(char *, int, struct proposal *, char **);

void	priv_tell_unwind(int, int, int, struct unwind_info *);

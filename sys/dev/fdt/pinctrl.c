/*	$OpenBSD: pinctrl.c,v 1.1 2018/08/26 19:50:08 kettenis Exp $	*/
/*
 * Copyright (c) 2018 Mark Kettenis <kettenis@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/malloc.h>

#include <machine/intr.h>
#include <machine/bus.h>
#include <machine/fdt.h>

#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_misc.h>
#include <dev/ofw/ofw_pinctrl.h>
#include <dev/ofw/fdt.h>

#define HREAD4(sc, reg)							\
	(bus_space_read_4((sc)->sc_iot, (sc)->sc_ioh, (reg)))
#define HWRITE4(sc, reg, val)						\
	bus_space_write_4((sc)->sc_iot, (sc)->sc_ioh, (reg), (val))

struct pinctrl_softc {
	struct device		sc_dev;
	bus_space_tag_t		sc_iot;
	bus_space_handle_t	sc_ioh;

	uint32_t		sc_reg_width;
	uint32_t		sc_func_mask;
};

int	pinctrl_match(struct device *, void *, void *);
void	pinctrl_attach(struct device *, struct device *, void *);

struct cfattach	pinctrl_ca = {
	sizeof (struct pinctrl_softc), pinctrl_match, pinctrl_attach
};

struct cfdriver pinctrl_cd = {
	NULL, "pinctrl", DV_DULL
};

int	pinctrl_pinctrl(uint32_t, void *);

int
pinctrl_match(struct device *parent, void *match, void *aux)
{
	struct fdt_attach_args *faa = aux;

	return OF_is_compatible(faa->fa_node, "pinctrl-single");
}

void
pinctrl_attach(struct device *parent, struct device *self, void *aux)
{
	struct pinctrl_softc *sc = (struct pinctrl_softc *)self;
	struct fdt_attach_args *faa = aux;

	if (faa->fa_nreg < 1) {
		printf(": no registers\n");
		return;
	}

	sc->sc_reg_width = OF_getpropint(faa->fa_node,
	    "pinctrl-single,register-width", 0);
	if (sc->sc_reg_width != 32) {
		printf(": unsupported register width\n");
		return;
	}

	sc->sc_func_mask = OF_getpropint(faa->fa_node,
	    "pinctrl-single,function-mask", 0);
	if (sc->sc_func_mask == 0) {
		printf(": bogus function mask\n");
		return;
	}

	sc->sc_iot = faa->fa_iot;
	if (bus_space_map(sc->sc_iot, faa->fa_reg[0].addr,
	    faa->fa_reg[0].size, 0, &sc->sc_ioh)) {
		printf(": can't map registers\n");
		return;
	}

	pinctrl_register(faa->fa_node, pinctrl_pinctrl, sc);

	printf("\n");
}

int
pinctrl_pinctrl(uint32_t phandle, void *cookie)
{
	struct pinctrl_softc *sc = cookie;
	uint32_t *pins;
	int node, len, i;
	
	node = OF_getnodebyphandle(phandle);
	if (node == 0)
		return -1;

	len = OF_getproplen(node, "pinctrl-single,pins");
	if (len <= 0)
		return -1;

	pins = malloc(len, M_TEMP, M_WAITOK);
	OF_getpropintarray(node, "pinctrl-single,pins", pins, len);

	for (i = 0; i < len / sizeof(uint32_t); i += 2) {
		uint32_t reg = pins[i];
		uint32_t func = pins[i + 1];
		uint32_t val;

		switch (sc->sc_reg_width) {
		case 32:
			val = HREAD4(sc, reg);
			val &= ~sc->sc_func_mask;
			val |= (func & sc->sc_func_mask);
			HWRITE4(sc, reg, val);
			break;
		}
	}

	free(pins, M_TEMP, len);
	return 0;
}

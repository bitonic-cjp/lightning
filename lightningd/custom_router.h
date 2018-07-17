/* Application-specific transaction routing */
#ifndef LIGHTNING_LIGHTNINGD_CUSTOM_ROUTER_H
#define LIGHTNING_LIGHTNINGD_CUSTOM_ROUTER_H
#include "config.h"
#include <common/htlc_wire.h>
#include <common/sphinx.h>
#include <lightningd/lightningd.h>

struct htlc_in;

void custom_route_payment(
	enum onion_type *failcode,
	const struct htlc_in *hin,
	const struct route_step *rs);

void custom_router_setup_connection(struct lightningd *ld, const char *filename);

#endif /* LIGHTNING_LIGHTNINGD_CUSTOM_ROUTER_H */

/* Application-specific transaction routing */
#ifndef LIGHTNING_LIGHTNINGD_CUSTOM_ROUTER_H
#define LIGHTNING_LIGHTNINGD_CUSTOM_ROUTER_H
#include "config.h"
#include <common/htlc_wire.h>

void custom_route_payment(enum onion_type *failcode, u8 realm, struct onionpacket *op);

#endif /* LIGHTNING_LIGHTNINGD_CUSTOM_ROUTER_H */

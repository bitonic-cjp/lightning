#include <lightningd/custom_router.h>

void custom_route_payment(enum onion_type *failcode, u8 realm, struct onionpacket *op)
{
	/* FIXME: implement this function */

	*failcode = WIRE_INVALID_REALM;
}


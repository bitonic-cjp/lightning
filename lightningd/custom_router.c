#include <lightningd/custom_router.h>

void custom_route_payment(
	enum onion_type *failcode,
	const struct htlc_in *hin,
	const struct route_step *rs)
{
	/* FIXME: implement this function */

	*failcode = WIRE_INVALID_REALM;
}


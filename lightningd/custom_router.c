#include <ccan/err/err.h>
#include <lightningd/channel.h>
#include <lightningd/custom_router.h>
#include <lightningd/htlc_end.h>
#include <lightningd/log.h>
#include <lightningd/peer_control.h>
#include <sys/types.h>
#include <unistd.h>

struct custom_router {
	struct lightningd *ld;
	int fd;
};

void custom_route_payment(
	enum onion_type *failcode,
	const struct htlc_in *hin,
	const struct route_step *rs)
{
	struct lightningd *ld = hin->key.channel->peer->ld;
	struct custom_router *router = ld->custom_router;

	if (!router) {
		log_debug(ld->log, "Custom router is not active: rejecting the payment");
		*failcode = WIRE_INVALID_REALM;
		return;
	}

	log_debug(ld->log, "Using custom router to handle the payment");

	//FIXME: write request to connection
	*failcode = WIRE_INVALID_REALM;
}

void custom_router_setup_connection(struct lightningd *ld, const char *filename)
{
	struct custom_router *router;

	struct sockaddr_un addr;
	int fd;

	if (!filename || streq(filename, ""))
		return;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		errx(1, "domain socket creation failed");
	}
	if (strlen(filename) + 1 > sizeof(addr.sun_path))
		errx(1, "filename '%s' too long", filename);
	strcpy(addr.sun_path, filename);
	addr.sun_family = AF_UNIX;

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
		errx(1, "Connect to '%s' failed", filename);

	log_debug(ld->log, "Connected to custom router on '%s'", filename);

	router = tal(ld, struct custom_router);
	router->ld = ld;
	router->fd = fd;

	//Register router in ld
	ld->custom_router = router;
}


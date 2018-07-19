#include <ccan/err/err.h>
#include <ccan/tal/str/str.h>
#include <errno.h>
#include <lightningd/channel.h>
#include <lightningd/custom_router.h>
#include <lightningd/htlc_end.h>
#include <lightningd/log.h>
#include <lightningd/peer_control.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

struct custom_router {
	struct lightningd *ld;
	int fd;
};

static bool write_all(int fd, const void *buf, size_t count)
{
	while (count) {
		ssize_t written = write(fd, buf, count);
		if(written < 0) return false;
		buf += written;
		count -= written;
	}
	return true;
}

static enum onion_type read_app_response(const struct custom_router *router)
{
	//FIXME: parse response from connection

	//Dummy read: continue until closing }
	char c = '\0';
	while(true) {
		read(router->fd, &c, 1);
		if(c == '}') break;
	}

	return WIRE_INVALID_REALM;
}

void custom_route_payment(
	enum onion_type *failcode,
	const struct htlc_in *hin,
	const struct route_step *rs)
{
	struct lightningd *ld = hin->key.channel->peer->ld;
	struct custom_router *router = ld->custom_router;
	char *command;

	if (!router) {
		log_debug(ld->log, "Custom router is not active: rejecting the payment");
		*failcode = WIRE_INVALID_REALM;
		return;
	}

	log_debug(ld->log, "Using custom router to handle the payment");

	// Write the command to the socket
	command = tal_fmt(tmpctx,
		"{"
		"\"method\": \"handle_payment\", "
		"\"params\": {"
			"\"realm\": %d"
		"}, "
		"\"id\": 0}",
		rs->hop_data.realm
		);

	if (!write_all(router->fd, command, strlen(command))) {
		//FIXME: proper handling, e.g. closing the connection
		log_debug(ld->log,
			"Failed to write command to the custom router: error %d (%s)",
			errno, strerror(errno));
		*failcode = WIRE_INVALID_REALM;
		return;
	}

	*failcode = read_app_response(router);
	if (*failcode) {
		log_debug(ld->log,
			"Custom router rejected the payment with code %d", *failcode);
	} else {
		log_debug(ld->log, "Custom router accepted the payment");
	}
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


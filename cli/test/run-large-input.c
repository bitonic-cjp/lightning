#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int test_main(int argc, char *argv[]);
ssize_t test_read(int fd, void *buf, size_t len);
int test_socket(int domain, int type, int protocol);
int test_connect(int sockfd, const struct sockaddr *addr,
		 socklen_t addrlen);
int test_getpid(void);
int test_printf(const char *format, ...);

#define main test_main
#define read test_read
#define socket test_socket
#define connect test_connect
#define getpid test_getpid
#define printf test_printf

  #include "../lightning-cli.c"
#undef main

/* AUTOGENERATED MOCKS START */
/* Generated stub for version_and_exit */
char *version_and_exit(const void *unused UNNEEDED)
{ fprintf(stderr, "version_and_exit called!\n"); abort(); }
/* AUTOGENERATED MOCKS END */

int test_socket(int domain UNUSED, int type UNUSED, int protocol UNUSED)
{
	/* We give a real fd, as it writes to it */
	return open("/dev/null", O_WRONLY);
}

int test_connect(int sockfd UNUSED, const struct sockaddr *addr UNUSED,
		 socklen_t addrlen UNUSED)
{
	return 0;
}

int test_getpid(void)
{
	return 9999;
}

int test_printf(const char *fmt UNUSED, ...)
{
	return 0;
}

static char *response;
static size_t response_off, max_read_return;

ssize_t test_read(int fd UNUSED, void *buf, size_t len)
{
	if (len > max_read_return)
		len = max_read_return;
	if (len > strlen(response + response_off))
		len = strlen(response + response_off);

	memcpy(buf, response + response_off, len);
	response_off += len;
	return len;
}

/* Simulate a real log file I captured */
#define NUM_ENTRIES (137772/2)

#define HEADER "{ \"jsonrpc\": \"2.0\",\n"				\
	       "    \"id\": \"lightning-cli-9999\",\n"			\
	       "    \"result\" : {\n"					\
	       "        \"creation_time\" : \"1515999039.806099043\",\n" \
	       "        \"bytes_used\" : 10787759,\n"			\
	       "        \"bytes_max\" : 20971520,\n"			\
	       "        \"log\" : [\n"
#define LOG_ENTRY							\
	"            {\"type\": \"SKIPPED\", \"num_skipped\": 22},\n"	\
	"            {\"type\": \"DEBUG\", \"time\": \"241693.051558854\", \"source\": \"lightning_gossipd(14581):\", \"log\": \"TRACE: nonlocal_gossip_broadcast_done\"},\n"
#define TAILER	"] } }"

int main(int argc UNUSED, char *argv[])
{
	setup_locale();

	char *fake_argv[] = { argv[0], "--lightning-dir=/tmp/", "test", NULL };
	size_t i;

	/* sizeof() is an overestimate, but we don't care. */
	response = tal_arr(NULL, char,
			   sizeof(HEADER)
			   + sizeof(LOG_ENTRY) * NUM_ENTRIES
			   + sizeof(TAILER));

	strcpy(response, HEADER);
	response_off = strlen(HEADER);

	/* Append a huge log */
	for (i = 0; i < NUM_ENTRIES; i++) {
		memcpy(response + response_off, LOG_ENTRY, sizeof(LOG_ENTRY)-1);
		response_off += sizeof(LOG_ENTRY)-1;
	}

	memcpy(response + response_off, TAILER, sizeof(TAILER)-1);
	response_off += sizeof(TAILER)-1;
	response[response_off++] = '\0';
	assert(strlen(response) == response_off - 1);
	assert(response_off < tal_len(response));

	response_off = 0;
	max_read_return = -1;
	assert(test_main(3, fake_argv) == 0);
	tal_free(response);
	return 0;
}

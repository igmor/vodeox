/* For sockaddr_in */
#include <netinet/in.h>
/* For socket functions */
#include <sys/socket.h>
/* For fcntl */
#include <fcntl.h>
#include <arpa/inet.h>

#include <event2/event.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

void do_read(evutil_socket_t fd, short events, void *arg);
void do_write(evutil_socket_t fd, short events, void *arg);

void print_ip(struct sockaddr_in& saddr)
{
    fprintf(stderr, "cli addr: %d.%d.%d.%d\n", (ntohl(saddr.sin_addr.s_addr) & 0xff000000) >> 24, 
            (ntohl(saddr.sin_addr.s_addr) & 0x00ff0000) >> 16, 
            (ntohl(saddr.sin_addr.s_addr) & 0x0000ff00) >> 8,
            (ntohl(saddr.sin_addr.s_addr) & 0x000000ff));
}

char
rot13_char(char c)
{
    /* We don't want to use isalpha here; setting the locale would change
     * which characters are considered alphabetical. */
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

struct fd_state {
    char buffer[MAX_LINE];
    size_t buffer_used;

    size_t n_written;
    size_t write_upto;

    struct event *read_event;
    struct event *write_event;

    sockaddr_in cli;
};

struct fd_state *
alloc_fd_state(struct event_base *base, evutil_socket_t fd)
{
    struct fd_state *state = (fd_state*)malloc(sizeof(struct fd_state));
    if (!state)
        return NULL;
    state->read_event = event_new(base, fd, EV_ET|EV_READ|EV_PERSIST, do_read, state);
    if (!state->read_event) {
        free(state);
        return NULL;
    }
    state->write_event =
        event_new(base, fd, EV_ET|EV_WRITE|EV_PERSIST, do_write, state);

    if (!state->write_event) {
        event_free(state->read_event);
        free(state);
        return NULL;
    }

    state->buffer_used = state->n_written = state->write_upto = 0;

    assert(state->write_event);
    return state;
}

void
free_fd_state(struct fd_state *state)
{
    event_free(state->read_event);
    event_free(state->write_event);
    free(state);
}

void
do_read(evutil_socket_t fd, short events, void *arg)
{
    fprintf(stderr, "%s\n", __FUNCTION__);

    struct fd_state *state = (fd_state*)arg;
    char buf[1024];
    int i;
    ssize_t result;
    struct sockaddr_in cli;
    socklen_t slen = sizeof(cli);

    while (1) {
        assert(state->write_event);
        memset(&cli, 0, sizeof(cli));
        result = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&cli, &slen);

        print_ip(cli);

        if (result <= 0)
            break;

        memcpy(&state->cli, &cli, sizeof(cli));
        for (i=0; i < result; ++i)  {
            if (state->buffer_used < sizeof(state->buffer))
                state->buffer[state->buffer_used++] = rot13_char(buf[i]);
            if (i == result-1) {
                assert(state->write_event);
                event_add(state->write_event, NULL);
                state->write_upto = state->buffer_used;
            }
        }
    }

    if (result == 0) {
        free_fd_state(state);
    } else if (result < 0) {
        if (errno == EAGAIN) // XXXX use evutil macro
            return;
        perror("recv");
        free_fd_state(state);
    }
}

void
do_write(evutil_socket_t fd, short events, void *arg)
{
    fprintf(stderr, "%s\n", __FUNCTION__);

    struct fd_state *state = (fd_state*)arg;

    while (state->n_written < state->write_upto) {
        ssize_t result = sendto(fd, state->buffer + state->n_written,
                                state->write_upto - state->n_written, 0, (struct sockaddr*)&(state->cli), sizeof(state->cli));
        if (result < 0) {
            print_ip(state->cli);
            fprintf(stderr, "sendto error %s %d %d\n", state->buffer, state->write_upto, errno);
            if (errno == EAGAIN) // XXX use evutil macro
                return;
            free_fd_state(state);
            return;
        }
        assert(result != 0);

        state->n_written += result;
    }

    if (state->n_written == state->buffer_used)
        state->n_written = state->write_upto = state->buffer_used = 1;

    event_del(state->write_event);
}

void
do_accept(evutil_socket_t listener, short event, void *arg)
{
    struct event_base *base = (event_base*)arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);

    int fd = accept(listener, (struct sockaddr*)&ss, &slen);

    if (fd < 0) 
    { // XXXX eagain??
        perror("accept");
    }
    else if (fd > FD_SETSIZE) 
    {
        close(fd); // XXX replace all closes with EVUTIL_CLOSESOCKET */
    } 
    else 
    {
        struct fd_state *state;
        evutil_make_socket_nonblocking(fd);
        state = alloc_fd_state(base, fd);

        assert(state); /*XXX err*/
        assert(state->write_event);
        
        event_add(state->read_event, NULL);
    }
}

void
run(void)
{
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base *base;
    struct event *listener_event;

    event_enable_debug_mode();

    base = event_base_new();
    if (!base)
        return; /*XXXerr*/

    memset(&sin,0,sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(40713);

    listener = socket(AF_INET, SOCK_DGRAM, 0);
    evutil_make_socket_nonblocking(listener);

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    struct fd_state *state;
    state = alloc_fd_state(base, listener);
        
    assert(state); /*XXX err*/
    assert(state->write_event);
    
    event_add(state->read_event, NULL);

    //run even loop
    event_base_dispatch(base);
}

int
main(int c, char **v)
{
    //    setvbuf(stdout, NULL, _IONBF, 0);

    run();
    return 0;
}



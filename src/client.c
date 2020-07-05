#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <systemd/sd-daemon.h>
#include <string.h>
#include <gio/gio.h>
#include <glib.h>
#include <unistd.h>

#define SOCK_PATH "/run/socketTest.sk"
#define MAX_LEN 2048

gboolean callback(GIOChannel *io, int condition, void *data)
{
        int client_fd = *(int *)data;
        int read_fd = g_io_channel_unix_get_fd(io);
        char buf[MAX_LEN] = {};
        int ret;
        int len;

        if (read(read_fd, buf, MAX_LEN) < 0) {
                perror("read error");
                close(client_fd);
                return FALSE;
        }

        len = strlen(buf);
        if (send(client_fd, buf, len + 1, 0) < 0) {
                perror("send error");
                close(client_fd);
                return FALSE;
        }
        printf("Sending data : %s\n", buf);
        if (recv(client_fd, buf, MAX_LEN, 0) < 0) {
                perror("recv error");
                close(client_fd);
                return FALSE;
        }

        printf("Receiving data : %s\n", buf);
        printf("\n>> ");
        fflush(stdout);
        return TRUE;
}
int main()
{
        int client_fd;
        struct sockaddr_un client_addr;
        GIOChannel *io;
        GMainLoop *loop;

        client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client_fd < 0)
        {
                perror("Create socket error");
                return 1;
        }

        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sun_family = AF_UNIX;
        strcpy(client_addr.sun_path, SOCK_PATH);

        if (connect(client_fd, (struct sockaddr *)&client_addr,
                    sizeof(client_addr)) < 0)
        {
                perror("connect error");
                return 1;
        }

        io = g_io_channel_unix_new(STDIN_FILENO);
        g_io_add_watch(io, G_IO_IN, (GIOFunc)callback, &client_fd);

        printf("Unix domain socket sample start\n");
        printf(">> ");
        fflush(stdout);

        loop = g_main_loop_new(NULL, FALSE);
        g_main_loop_run(loop);
        return 0;
}
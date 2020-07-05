#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <systemd/sd-daemon.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-unix.h>
#include <string.h>

#define SOCK_PATH "/run/socketTest.sk"
#define MAX_LEN 2048

gboolean receive_callback(GIOChannel *io, int cond, void *data)
{
        int client_fd = g_io_channel_unix_get_fd(io);
        int len;
        char buf[MAX_LEN];
        if (cond != G_IO_IN) {
                perror("Client disconnected");
                return FALSE;
        }

        if (recv(client_fd, buf, MAX_LEN, 0) < 0) {
                perror("recv error");
                return FALSE;
        }

        printf("Receiving data : %s\n", buf);
        
        strcpy(buf, "ack");
        if (send(client_fd, buf, 4, 0) < 0) {
                perror("send error");
                return FALSE;
        }

        return TRUE;
}
gboolean accept_callback(int fd, int cond, void *data)
{
        int client_fd;
        int ret;
        GIOChannel *io;
        printf("%d\n", cond);
        client_fd = accept(fd, NULL, NULL);
        if (client_fd < 0) {
                perror("accept error");
                close(fd);
                close(client_fd);
                return FALSE;
        }

        cond = G_IO_IN | G_IO_ERR | G_IO_HUP;
        io = g_io_channel_unix_new(client_fd);
        g_io_add_watch(io, cond, (GIOFunc)receive_callback, NULL);
        printf("Client conneted\n");
        return TRUE;
}
int main()
{
        int server_fd = -1;
        int cond;
        struct sockaddr_un server_addr;
        GMainLoop *loop;

        int n = sd_listen_fds(0);

        for (int i = SD_LISTEN_FDS_START; i < SD_LISTEN_FDS_START + n; ++i) {
                if (sd_is_socket_unix(i, AF_UNIX, SOCK_STREAM, 
                SOCK_PATH, 0) > 0) {
                        server_fd = i;
                        break;
                }
        }
        if (server_fd < 0) {
                server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
                if (server_fd < 0) {
                        perror("Create socket error");
                        return 1;
                }

                memset(&server_addr, 0, sizeof(server_addr));
                server_addr.sun_family = AF_UNIX;
                strcpy(server_addr.sun_path, SOCK_PATH);
                unlink(SOCK_PATH);
                if (bind(server_fd, (struct sockaddr *)&server_addr, 
                        sizeof(server_addr)) < 0) {
                        perror("bind error");
                        return 1;
                }

                if (listen(server_fd, 5) < 0) {
                        perror("listen error");
                        return 1;
                }
        }

        g_unix_fd_add(server_fd, G_IO_IN, (GUnixFDSourceFunc)accept_callback, NULL);

        loop = g_main_loop_new(NULL, FALSE);
        g_main_loop_run(loop);
        return 0;
}
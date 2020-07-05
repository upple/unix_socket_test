gcc src/server.c -o socketServer `pkg-config --cflags --libs glib-2.0 libsystemd`
gcc src/client.c -o socketClient `pkg-config --cflags --libs glib-2.0 libsystemd`

mv socketServer /usr/bin/
mv socketClient /usr/bin/
cp socketTest.service /etc/systemd/system/socketTest.service
cp socketTest.socket /etc/systemd/system/socketTest.socket

systemctl enable socketTest.socket
systemctl start socketTest.socket
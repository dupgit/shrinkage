# libmicrohttpd used is in my home dir at /home/dup/local/include (for *.h) and /home/dup/local/lib (for *.so)
gcc -o shrinkserver server.c -I/home/dup/local/include -L/home/dup/local/lib -lmicrohttpd -Wall
gcc -o shrinkclient client.c -Wall -lcurl

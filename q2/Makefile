CC = gcc
CFLAGS = -Wall -Wextra -O2
THREADS = -pthread

# Question 1
q1_server: q1/server.c
	$(CC) $(CFLAGS) $(THREADS) q1/server.c -o q1/server

q1_client: q1/client.c
	$(CC) $(CFLAGS) q1/client.c -o q1/client

q1: q1_server q1_client

# Question 2
q2_server: q2/server.c
	$(CC) $(CFLAGS) $(THREADS) q2/server.c -o q2/server

q2_client: q2/client.c
	$(CC) $(CFLAGS) q2/client.c -o q2/client

q2: q2_server q2_client

clean:
	rm -f q1/server q1/client q2/server q2/client

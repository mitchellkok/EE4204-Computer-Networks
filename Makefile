#Objects
HEADER = headsock.c

client_src = udp_client.c
server_src = udp_server.c

client_o = udp_client.o
server_o = udp_server.o

TARGET_C = udp_client
TARGET_S = udp_server
CC = gcc

all: $(TARGET_C) $(TARGET_S)

$(TARGET_C): $(client_src)
	$(CC) -o $@ $^ 

$(TARGET_S): $(server_src)
	$(CC) -o $@ $^

clean:
	rm -f $(TARGET_C) $(TARGET_S)
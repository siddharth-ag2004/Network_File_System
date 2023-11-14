CC = gcc
CFLAGS = -Wall -Wextra -pthread -Werror
LDFLAGS = -lm

CLIENT_SOURCES = socketAPI.c client.c
NMS_SOURCES = socketAPI.c nms.c trie.c

CLIENT_EXECUTABLE = client
NMS_EXECUTABLE = nms

all: $(CLIENT_EXECUTABLE) $(NMS_EXECUTABLE)

$(CLIENT_EXECUTABLE): $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(CLIENT_SOURCES) -o $(CLIENT_EXECUTABLE) $(LDFLAGS)

$(NMS_EXECUTABLE): $(NMS_SOURCES)
	$(CC) $(CFLAGS) $(NMS_SOURCES) -o $(NMS_EXECUTABLE) $(LDFLAGS)

clean:
	rm -f $(CLIENT_EXECUTABLE) $(NMS_EXECUTABLE)
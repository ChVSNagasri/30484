
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRC = $(filter-out src/file_links.c src/inode_info.c,$(wildcard src/*.c))

TARGET = shellforge

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -lreadline -o $(TARGET)

inode_info: src/inode_info.c
	$(CC) $(CFLAGS) $< -o $@

file_links: src/file_links.c
	$(CC) $(CFLAGS) $< -o $@

all: shellforge inode_info file_links

clean:
	rm -f shellforge inode_info file_links

.PHONY: all clean


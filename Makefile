CC = gcc
CFLAGS = -I$(LDIR) -g -Wall

.PHONY: all clean

SDIR = src
ODIR = $(SDIR)/obj
LDIR = lib

LIBS = -lSDL2 -lSDL2_ttf -lpthread

BOT_OBJ = bot.o board_library.o UI_library.o client_con.o client_com.o bot_game.o client_game.o
CLIENT_OBJ = client.o board_library.o UI_library.o client_con.o client_com.o client_game.o
SERVER_OBJ = server.o board_library.o server_con.o server_com.o server_game.o

BOBJ = $(patsubst %,$(ODIR)/%,$(BOT_OBJ))
COBJ = $(patsubst %,$(ODIR)/%,$(CLIENT_OBJ))
SOBJ = $(patsubst %,$(ODIR)/%,$(SERVER_OBJ))

all: bot client server

bot : $(BOBJ)
	$(CC) -o $@ $^ $(LIBS)

client : $(COBJ)
	$(CC) -o $@ $^ $(LIBS)

server : $(SOBJ)
	$(CC) -o $@ $^ $(LIBS)

$(ODIR):
	mkdir -p $@

SRC = bot
LIB = board_library.h UI_library.h connection.h communication.h bot_game.h client_game.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = client
LIB = board_library.h UI_library.h connection.h communication.h client_game.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = server
LIB = board_library.h connection.h communication.h server_game.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = board_library
LIB = board_library.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)

	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = UI_library
LIB = UI_library.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = client_con
LIB = connection.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = server_con
LIB = connection.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = server_com
LIB = communication.h board_library.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = client_com
LIB = communication.h board_library.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = server_game
LIB = server_game.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = client_game
LIB = client_game.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

SRC = bot_game
LIB = bot_game.h client_game.h
$(ODIR)/$(SRC).o : $(SDIR)/$(SRC).c $(patsubst %,$(LDIR)/%, $(LIB)) | $(ODIR)
	$(CC) -c -o $@ $(patsubst $(ODIR)%.o,$(SDIR)%.c,$@) $(CFLAGS)

clean:
	rm -rf bot client server $(ODIR)

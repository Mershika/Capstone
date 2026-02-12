CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
BUILD_DIR = build

# ==========================
# SERVER SOURCES
# ==========================

SERVER_SRCS = \
	src/Server/Server.cpp \
	src/Server/ClientHandler.cpp \
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS = -lcrypto
BUILD_DIR = build

SERVER_SRCS = \
	src/Server/Server.cpp \
	src/Server/ClientHandler.cpp \
	src/DirectoryTraverser/DirectoryTraverser.cpp \
	src/ContentScanner/ContentScanner.cpp \
	src/FileInspector/FileInspector.cpp \
	src/Logger/Logger.cpp \
	src/ExceptionHandler/ExceptionHandler.cpp \
	app/server_main.cpp

# ==========================
# CLIENT SOURCES
# ==========================


CLIENT_SRCS = \
	src/Client/Client.cpp \
	app/client_main.cpp

SERVER_BIN = $(BUILD_DIR)/server
CLIENT_BIN = $(BUILD_DIR)/client

all: directories server client

directories:
	mkdir -p $(BUILD_DIR)

server:
	$(CXX) $(CXXFLAGS) $(SERVER_SRCS) -o $(SERVER_BIN) $(LDFLAGS)

client:
	$(CXX) $(CXXFLAGS) $(CLIENT_SRCS) -o $(CLIENT_BIN)

clean:
	rm -rf $(BUILD_DIR)

run-server: server
	./$(SERVER_BIN)

run-client: client
	./$(CLIENT_BIN)

.PHONY: all clean server client directories run-server run-client

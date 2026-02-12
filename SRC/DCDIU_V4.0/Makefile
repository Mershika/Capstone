# ============================================================
# Makefile for DCDIU (Directory Content Detection & Inspection Utility)
# ============================================================
#
# Project      : DCDIU v4.0
# Description  : Build configuration for multi-threaded server-client
#                directory traversal and content scanning application.
#
# Build Targets:
#   - make          : Build both server and client
#   - make server   : Build server binary only
#   - make client   : Build client binary only
#   - make clean    : Remove build directory
#   - make run-server : Build and run server
#   - make run-client : Build and run client
#
# Compiler Configuration:
#   - C++ Standard : C++17
#   - Warnings     : Wall, Wextra (strict compilation)
#   - Libraries    : OpenSSL (libcrypto for SHA-256 hashing)
#
# ============================================================

# Compiler and compilation flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# Linker flags - includes OpenSSL library for cryptographic operations
LDFLAGS = -lcrypto

# Output directory for compiled binaries
BUILD_DIR = build

# ============================================================
# SERVER SOURCES
# ============================================================
# List of all source files needed to build the server:
#   - Core server socket and connection handling
#   - Per-client session management
#   - Directory traversal operations
#   - File content scanning
#   - File inspection utilities
#   - Logging and exception handling
SERVER_SRCS = \
	src/Server/Server.cpp \
	src/Server/ClientHandler.cpp \
	src/DirectoryTraverser/DirectoryTraverser.cpp \
	src/ContentScanner/ContentScanner.cpp \
	src/FileInspector/FileInspector.cpp \
	src/Logger/Logger.cpp \
	src/ExceptionHandler/ExceptionHandler.cpp \
	app/server_main.cpp

# ============================================================
# CLIENT SOURCES
# ============================================================
# List of all source files needed to build the client:
#   - Client network communication module
#   - Logging utilities
#   - Client application entry point
CLIENT_SRCS = \
	src/Client/Client.cpp \
	src/Logger/Logger.cpp \
	app/client_main.cpp

# Output binary paths
SERVER_BIN = $(BUILD_DIR)/server
CLIENT_BIN = $(BUILD_DIR)/client

# ============================================================
# BUILD TARGETS
# ============================================================

# Default target: ensure build directory exists, then build both binaries
all: directories server client

# Create build directory if it doesn't exist
directories:
	mkdir -p $(BUILD_DIR)

# Build server binary with OpenSSL library linking
server:
	$(CXX) $(CXXFLAGS) $(SERVER_SRCS) -o $(SERVER_BIN) $(LDFLAGS)

# Build client binary
client:
	$(CXX) $(CXXFLAGS) $(CLIENT_SRCS) -o $(CLIENT_BIN)

# Remove all compiled binaries and build directory
clean:
	rm -rf $(BUILD_DIR)

# Build and execute server
run-server: server
	./$(SERVER_BIN)

# Build and execute client
run-client: client
	./$(CLIENT_BIN)

# Mark these targets as phony (not file-based)
.PHONY: all clean server client directories run-server run-client

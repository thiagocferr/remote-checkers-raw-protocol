
MAKE := make

# Path to where player files are stored
SERVER_DIR=server

all:
	$(MAKE) -C server all_central SERVER_DIR=$(SERVER_DIR)
	$(MAKE) -C client all_central


.PHONY: server
server:
	$(MAKE) -C server all_central SERVER_DIR=$(SERVER_DIR)

.PHONY: client
client:
	$(MAKE) -C client all_central

.PHONY: clean
clean:
	$(MAKE) -C server clean_central
	$(MAKE) -C client clean_central

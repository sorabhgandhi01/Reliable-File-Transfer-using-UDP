SUBDIRS = client server
SRC = client server

.PHONY: all $(SUBDIRS)
all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ -f Makefile

.PHONY: clean $(SRC)
clean: $(SRC)
$(SRC): 
	$(MAKE) -C $@ -f Makefile


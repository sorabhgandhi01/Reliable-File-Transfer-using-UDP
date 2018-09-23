SUBDIRS = client server

#.PHONY: all $(SUBDIRS)
#all: $(SUBDIRS)
#$(SUBDIRS):
#	$(MAKE) -C $@ -f Makefile

.PHONY: all clean

all clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir -f Makefile $@; \
	done

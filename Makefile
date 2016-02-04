
SUBDIRS         = blink ledblink simple test1 rctopi

.PHONY: all clean

all clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done


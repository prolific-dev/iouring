.PHONY: all clean

all:
	$(MAKE) -C src/util
	$(MAKE) -C src/nat

clean:
	$(MAKE) -C src/util clean
	$(MAKE) -C src/nat clean
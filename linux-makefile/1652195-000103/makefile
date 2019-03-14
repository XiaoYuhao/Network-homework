exclude_dirs:=include bin
 
dirs:=$(shell find . -maxdepth 1 -type d)
dirs:=$(basename $(patsubst ./%,%,$(dirs)))
dirs:=$(filter-out $(exclude_dirs),$(dirs))
 
.PHONY: $(dirs) clean
 
$(dirs):
	@for dir in $(dirs); do \
		$(MAKE) -C $$dir; \
	done
 
clean:
	@for dir in $(dirs); do \
		$(MAKE) -C $$dir clean; \
	done

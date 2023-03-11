SUBDIRS = datacorruptor datacreator datareader common

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
	
clean:
	for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done



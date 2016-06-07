include build/rules.mk

SUBDIRS = 	kva-tool	\
		mmsearch	\
		mmsearch/test	\

all: subdirs

clean:
	@for dir in $(SUBDIRS) ; do \
        if [ -d $$dir ]; then ( cd $$dir ; make clean ) ; fi \
    done

include build/common.mk











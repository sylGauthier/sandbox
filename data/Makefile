BLEND_SOURCE="syg@vps:/home/syg/storage/sandbox/blends"
TEX_SOURCE="syg@vps:/home/syg/storage/sandbox/textures"
OGEX_SOURCE="syg@vps:/home/syg/storage/sandbox/ogex"

FINAL_OGEX := $(patsubst blends/%.blend,ogex/%.ogex, $(wildcard blends/*.blend))

.PHONY: all

all: $(FINAL_OGEX)

blends textures ogex:
	mkdir "$@"

ogex/%.ogex: blends/%.blend ogex
	../scripts/blend2ogex.sh "$<" "$@"

clean:
	rm ogex/*.ogex blends/*.ogex

push_blends:
	[ -n "$(BLEND_SOURCE)" ] && rsync -P blends/*.blend $(BLEND_SOURCE)

push_textures:
	[ -n "$(TEX_SOURCE)" ] && rsync -rP textures/ $(TEX_SOURCE)

push_ogex:
	[ -n "$(OGEX_SOURCE)" ] && rsync -rzP ogex/ $(OGEX_SOURCE)

push_all: push_blends push_textures push_ogex

pull_blends: blends
	[ -n "$(BLEND_SOURCE)" ] && rsync -rP $(BLEND_SOURCE) ./

pull_textures: textures
	[ -n "$(TEX_SOURCE)" ] && rsync -rP $(TEX_SOURCE) ./

pull_ogex: ogex
	[ -n "$(OGEX_SOURCE)" ] && rsync -rzP $(OGEX_SOURCE) ./

pull_all: pull_blends pull_textures pull_ogex


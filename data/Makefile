BLEND_SOURCE="syg@vps:/home/syg/storage/sandbox/blends"
TEX_SOURCE="syg@vps:/home/syg/storage/sandbox/textures"
GLB_SOURCE="syg@vps:/home/syg/storage/sandbox/glb"

FINAL_GLB := $(patsubst blends/%.blend,glb/%.glb, $(wildcard blends/*.blend))

.PHONY: all

all: $(FINAL_GLB)

blends textures glb:
	mkdir "$@"

glb/%.glb: blends/%.blend glb
	../scripts/blend2glb.sh "$<" "$@"

clean:
	rm glb/*.glb

push_blends:
	[ -n "$(BLEND_SOURCE)" ] && rsync -P blends/*.blend $(BLEND_SOURCE)

push_textures:
	[ -n "$(TEX_SOURCE)" ] && rsync -rP textures/ $(TEX_SOURCE)

push_glb:
	[ -n "$(GLB_SOURCE)" ] && rsync -rzP glb/ $(GLB_SOURCE)

push_all: push_blends push_textures push_glb

pull_blends: blends
	[ -n "$(BLEND_SOURCE)" ] && rsync -rP $(BLEND_SOURCE) ./

pull_textures: textures
	[ -n "$(TEX_SOURCE)" ] && rsync -rP $(TEX_SOURCE) ./

pull_glb: glb
	[ -n "$(GLB_SOURCE)" ] && rsync -rzP $(GLB_SOURCE) ./

pull_all: pull_blends pull_textures pull_glb


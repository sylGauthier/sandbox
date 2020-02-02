FINAL_OGEX := $(patsubst blends/%.blend,ogex/%.ogex, $(wildcard blends/*.blend))

.PHONY: all

all: $(FINAL_OGEX)

ogex/%.ogex: blends/%.blend
	../scripts/blend2ogex.sh "$<" "$@"

clean:
	rm ogex/*.ogex blends/*.ogex

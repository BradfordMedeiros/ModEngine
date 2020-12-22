# TODO -> better if this put the exported files to be not in source but rather relative in a build directory

SOURCES := $(wildcard ../../../gameresources/**/*.blend)
OBJ :=$(patsubst %.blend, %.fbx, ${SOURCES})

all: $(OBJ)

%.fbx: %.blend
	@blender $< --background --python ./export-model.py -- $@

clean:
	@rm -f ${OBJ}
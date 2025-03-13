

C_BASE_SRCS= ascui.c texts.c raytiles.c common.c
C_BASE_DIR=src/c_codebase/src
C_CODEBASE=$(addprefix $(C_BASE_DIR)/,$(C_BASE_SRCS))
HGSS=src/map.c src/mapgen.c
MAIN=src/main.c

_main:
	gcc $(MAIN) $(C_CODEBASE) $(HGSS) -g -lc -lm -lraylib -I /usr/local/include -o bin/main
main: _main
	./bin/main



C_BASE_SRCS= ascui.c texts.c raytiles.c common.c
C_BASE_DIR=src/c_codebase/src
C_CODEBASE=$(addprefix $(C_BASE_DIR)/,$(C_BASE_SRCS))
HGSS=src/map.c src/mapgen.c src/world.c
MAIN=src/main/hgss.c
EDITOR=src/editor/hgss_editor.c

clean: 
	$(RM) -r bin/*
	
_hgss: clean
	gcc $(MAIN) $(C_CODEBASE) $(HGSS) -g -lc -lm -lraylib -I /usr/local/include -o bin/hgss
hgss: _hgss
	./bin/hgss
	
vg_hgss: _hgss
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./bin/hgss

_editor: clean
	gcc $(EDITOR) $(C_CODEBASE) $(HGSS) -g -lc -lm -lraylib -I /usr/local/include -o bin/hgss_editor
editor: _editor
	./bin/hgss_editor

vg_editor: _editor
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./bin/hgss_editor

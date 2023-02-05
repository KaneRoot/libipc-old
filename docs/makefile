all: get-point-tools clean generate display

BAGUETTE=https://git.baguette.netlib.re/Baguette
get-point-tools:
	@test -d catpoint || (git clone $(BAGUETTE)/catpoint.git && cd catpoint && make)
	@test -d pointtools || (git clone $(BAGUETTE)/pointtools.git && cd pointtools && make)

CATPOINT = $(PWD)/catpoint/catpoint
MD2POINT = $(PWD)/pointtools/bin/md2point
PRESENTATION = $(PWD)/libipc.md

BDIR=tmp
clean:
	@-rm $(BDIR)/*.txt 2>/dev/null || true

generate:
	@cd $(BDIR) && (cat $(PRESENTATION) | $(MD2POINT))

display:
	@cd $(BDIR) && $(CATPOINT) *.txt

help:
	@echo "get-point-tools: get all relevant point tools (catpoint and md2point)"
	@echo
	@echo "generate: convert markdown into 'point' documents"
	@echo "display:  run catpoint on all 'point' document"
	@echo "clean:    remove all 'point' files"
	@echo
	@echo "By default: get point tools, generate then display the presentation"

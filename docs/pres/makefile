all: clean generate display

clean:
	-rm ./*.txt 2>/dev/null || true

generate:
	cat ../libipc.md | md2point

display:
	catpoint *.txt

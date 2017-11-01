all: release

auto:
	./make_auto

debug:
	./make_auto
	make debug

release:
	./make_auto
	make release

clean:
	echo "Workspace is clean"


all: compile link

compile:
	cd massivereader && make
	cd multiwriter && make

link: compile
	mkdir -p ODP
	ln -s massivereader/massivereader ODP/massivereader
	ln -s multiwriter/multiwriter ODP/multiwriter

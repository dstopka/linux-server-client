all: compile link

compile: clean
	cd massivereader && make
	cd multiwriter && make

link: compile
	mkdir -p ODP
	ln -s ../massivereader/massivereader ODP/massivereader
	ln -s ../multiwriter/multiwriter ODP/multiwriter
	
clean:
	rm -f ODP/massivereader ODP/multiwriter

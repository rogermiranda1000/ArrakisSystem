all:
	make -C Fremen
	make -C Atreides
	make -C Harkonen

clean:
	make clean -C Fremen
	make clean -C Atreides
	make clean -C Harkonen
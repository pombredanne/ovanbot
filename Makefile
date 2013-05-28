bot:
	make -C src
	@cp src/bot .

.PHONY: clean
clean:
	rm -f bot
	make -C src clean

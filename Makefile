.PHONY: bot
bot:
	make -C src bot
	@cp src/bot .

.PHONY: clean
clean:
	rm -f bot
	make -C src clean

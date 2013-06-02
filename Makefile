.PHONY: bot
bot:
	make -C src bot
	@cp src/bot .

.PHONY: protobuf
protobuf:
	protoc --cpp_out=src protobuf/ovanbot.proto

.PHONY: clean
clean:
	rm -f bot
	make -C src clean

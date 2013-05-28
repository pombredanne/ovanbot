CFLAGS := -std=c++11 -g -pthread
LIBS := -lboost_system -lboost_program_options -lre2

.PHONY: all
all: bot

bot.o: bot.cc
	$(CXX) $(CFLAGS) -c $<

bot: bot.o main.cc
	$(CXX) $(CFLAGS) $^ $(LIBS) -o $@

.PHONY: clean
clean:
	-rm -f bot.o bot

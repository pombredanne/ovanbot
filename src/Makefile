CFLAGS := -std=c++11 -g -pthread
LIBS := -lboost_system -lboost_program_options -lre2
SRCS := bot.cc bot.h

.PHONY: all
all: bot

%.o: %.cc %.h
	$(CXX) $(CFLAGS) -c $<

bot: bot.o plugin_karma.o plugin_logging.o plugin_owner.o plugin.o main.cc
	$(CXX) $(CFLAGS) $^ $(LIBS) -o $@

.PHONY: clean
clean:
	-rm -f *.o bot

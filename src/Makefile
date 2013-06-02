CFLAGS := -std=c++11 -g -pthread -I.
LIBS := -lboost_system -lboost_program_options -lprotobuf -lre2
SRCS := bot.cc bot.h

.PHONY: all
all: bot

%.o: %.cc %.h
	$(CXX) $(CFLAGS) -c $< -o $@

bot: bot.o config.o protobuf/ovanbot.pb.o plugin_karma.o plugin_logging.o plugin_owner.o main.cc
	$(CXX) $(CFLAGS) $^ $(LIBS) -o $@

.PHONY: clean
clean:
	-rm -f *.o bot

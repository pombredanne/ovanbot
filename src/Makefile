CFLAGS := -std=c++11 -g -pthread
LIBS := -lboost_system -lboost_program_options -lre2
SRCS := bot.cc bot.h

.PHONY: all
all: bot

bot.o: bot.cc bot.h
	$(CXX) $(CFLAGS) -c $<

logfile.o: logfile.cc logfile.h
	$(CXX) $(CFLAGS) -c $<

bot: bot.o logfile.o main.cc
	$(CXX) $(CFLAGS) $^ $(LIBS) -o $@

.PHONY: clean
clean:
	-rm -f *.o bot

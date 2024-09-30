SRCS = mozart.cpp Parser.cpp Lexer.cpp
TARGET = mozart

CXX = g++
CXXFLAGS = -std=c++20


all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++23

BUILD := build
EXAMPLES = ex1 ex2 ex3 ex4 ex5

all: $(EXAMPLES)

$(EXAMPLES):
	mkdir -p $(BUILD)/src
	$(CXX) $(CXXFLAGS) -O2 -I ./include -o $(BUILD)/$@ src/$@.cc

.PHONY: clean
clean:
	rm -r $(BUILD)

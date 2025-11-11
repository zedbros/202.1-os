TARGET := main

BUILD_DIR := build
SRC := $(wildcard src/*.cc)
OBJ := $(SRC:%=build/%.o)

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++23

all: $(BUILD_DIR)/main

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cc.o: %.cc
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I ./include -o $@ -c $<

.PHONY: test
test:
	$(CXX) $(CXXFLAGS) -I ./include -o $(BUILD_DIR)/test-all test/test-all.cc
	$(BUILD_DIR)/test-all

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

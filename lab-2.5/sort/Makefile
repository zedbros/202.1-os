TARGET := main

BUILD_DIR := build
SRC := $(shell find src -name *.cc)
OBJ := $(SRC:%=build/%.o)

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20

all: $(BUILD_DIR)/main

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cc.o: %.cc
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I ./include -o $@ -c $<

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

TARGET := main

INCLD_DIR := include
BUILD_DIR := build

SRC := $(wildcard src/*.cc)
OBJ := $(SRC:%=build/%.o)
SRC_TEST := $(filter-out src/main.cc, $(SRC))
OBJ_TEST := $(SRC_TEST:%=build/%.o)

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++23

all: $(BUILD_DIR)/main

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cc.o: %.cc
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I $(INCLD_DIR) -o $@ -c $<

.PHONY: test
test: $(OBJ_TEST)
	$(CXX) $(CXXFLAGS) -I $(INCLD_DIR) -o $(BUILD_DIR)/test-all $(OBJ_TEST) test/test-all.cc
	$(BUILD_DIR)/test-all

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

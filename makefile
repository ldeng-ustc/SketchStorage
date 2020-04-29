CXX      := g++
CXXFLAGS := --std=c++17
LDFLAGS  := -lpcap -lrocksdb -lpthread -lz -ldl

BUILD    := ./build
SRC_DIR  := ./src
MOD_DIR  := $(SRC_DIR)/modules
APP_DIR  := $(BUILD)/apps
OBJ_DIR  := $(BUILD)/objects
DAT_DIR  := ./data

INCLUDE  := $(wildcard $(SRC_DIR)/*.h) $(wildcard $(MOD_DIR)/*.h)
SRC      := $(wildcard $(SRC_DIR)/*.cpp)
MOD_SRC  := $(wildcard $(MOD_DIR)/*.cpp)
OBJECTS  := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(MOD_SRC)))
TARGETS	 := $(patsubst %.cpp, $(APP_DIR)/%, $(notdir $(SRC)))

EVAL_APP_DIR := $(APP_DIR)/eval
EVAL_SRC_DIR := $(SRC_DIR)/eval
EVAL_SRC     := $(wildcard $(EVAL_SRC_DIR)/*.cpp)
EVAL_TARGETS := $(patsubst %.cpp, $(EVAL_APP_DIR)/%, $(notdir $(EVAL_SRC)))

#MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
#MK_DIR  := $(dir $(MK_PATH))

.PHONY: all clean debug info

all: info build $(TARGETS) $(EVAL_TARGETS)

info:
	@echo mod_src: $(MOD_SRC)
	@echo objects: $(OBJECTS)
	@echo eval_src: $(EVAL_SRC)
	@echo eval_targets: $(EVAL_TARGETS) 
	@echo src: $(SRC)
	@echo targets: $(TARGETS)


$(OBJ_DIR)/%.o: $(MOD_DIR)/%.cpp $(INCLUDE)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

$(APP_DIR)/%: $(SRC_DIR)/%.cpp $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(EVAL_APP_DIR)/%: $(EVAL_SRC_DIR)/%.cpp $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
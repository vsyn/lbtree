# Copyright 2023 Julian Ingram
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DEFINES :=
INCLUDES = ./ $(TEST_DIR) $(TEST_DIR)/everand $(EXAMPLE_DIR)

CC := gcc
AR := ar
CFLAGS += -O3 -Werror -Wall -Wextra $(DEFINES:%=-D%) $(INCLUDES:%=-I%)
# expanded below
DEPFLAGS = -MMD -MP -MF $(@:$(BUILD_DIR)/%.o=$(DEP_DIR)/%.d)
LDFLAGS := -O3
SRCS := lbtree.c
TEST_DIR := tests
TEST_SRCS := \
	$(SRCS) \
	lbtree_d.c \
	$(TEST_DIR)/everand/everand.c \
	$(TEST_DIR)/tree_test.c \
	$(TEST_DIR)/lbtree_d_test.c \
	$(TEST_DIR)/lbtree_test.c \
	$(TEST_DIR)/tsearch_test.c \
	$(TEST_DIR)/main.c
EXAMPLE_DIR := examples
EXAMPLE_SRCS := $(SRCS) $(EXAMPLE_DIR)/lbtree_uint.c
# sort removes duplicates
ALL_SRCS := $(sort $(SRCS) $(TEST_SRCS))
BIN_DIR ?= bin
TARGET ?= $(BIN_DIR)/liblbtree.a
TEST ?= $(BIN_DIR)/lbtree_test
EXAMPLE ?= $(BIN_DIR)/liblbtree_uint.a
RM := rm -rf
MKDIR := mkdir -p
CP := cp -r
# BUILD_DIR and DEP_DIR should both have non-empty values
BUILD_DIR ?= build
DEP_DIR ?= $(BUILD_DIR)/deps
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)
EXAMPLE_OBJS := $(EXAMPLE_SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(ALL_SRCS:%.c=$(DEP_DIR)/%.d)

.PHONY: all
all: $(TARGET)

# link lib
$(TARGET): $(OBJS)
	$(if $(BIN_DIR),$(MKDIR) $(BIN_DIR),)
	$(AR) -rcsD $@ $^

.PHONY: test
test: $(TEST)
	./$(BIN_DIR)/lbtree_test

# link test
$(TEST): $(TEST_OBJS)
	$(if $(BIN_DIR),$(MKDIR) $(BIN_DIR),)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: example
example: $(EXAMPLE)

# link examples
$(EXAMPLE): $(EXAMPLE_OBJS)
	$(if $(BIN_DIR),$(MKDIR) $(BIN_DIR),)
	$(AR) -rcsD $@ $^

# compile and/or generate dep files
$(BUILD_DIR)/%.o: %.c
	$(MKDIR) $(BUILD_DIR)/$(dir $<)
	$(MKDIR) $(DEP_DIR)/$(dir $<)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(TEST) $(EXAMPLE) $(BIN_DIR) $(DEP_DIR) $(BUILD_DIR)

-include $(DEPS)

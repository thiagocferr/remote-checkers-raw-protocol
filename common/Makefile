
GCC       := g++
CPP_FLAGS := -std=c++17 -O0 -Wall -Wextra -pedantic

COMMON_INCLUDE_PATH := include/
COMMON_SRCDIR	  := src
COMMON_OBJDIR := bin
COMMON_SRC_FILES := $(wildcard $(COMMON_SRCDIR)/*.cpp)


COMMON_OBJ_FILES := $(COMMON_SRC_FILES:.cpp=.o)
COMMON_OBJ_FILES := $(addprefix $(COMMON_OBJDIR)/, $(COMMON_OBJ_FILES:$(COMMON_SRCDIR)/%=%))

all: $(COMMON_OBJ_FILES)

$(COMMON_OBJ_FILES): $(COMMON_OBJDIR)/%.o : $(COMMON_SRCDIR)/%.cpp | $(COMMON_OBJDIR)
	$(GCC) $(CPP_FLAGS) -I $(COMMON_INCLUDE_PATH) -c $^ -o $@

$(COMMON_OBJDIR):
	mkdir $@

clean:
	rm -rf bin
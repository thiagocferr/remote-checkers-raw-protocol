GCC       := g++
CPP_FLAGS := -std=c++17 -O0 -Wall -Wextra -pedantic


INCLUDE_PATH := include/
SRCDIR	  := src
OBJDIR    := bin
SRC_FILES := $(wildcard $(SRCDIR)/*.cpp)

OBJ_FILES := $(SRC_FILES:.cpp=.o)
OBJ_FILES := $(addprefix $(OBJDIR)/, $(OBJ_FILES:$(SRCDIR)/%=%))


COMMON_INCLUDE_PATH := ../common/include/
COMMON_SRC := ../common/src
COMMON_OBJDIR := ../common/bin

COMMON_OBJ_FILES := $(wildcard $(COMMON_SRC)/*.cpp)
COMMON_OBJ_FILES := $(patsubst %.cpp,%.o, $(COMMON_OBJ_FILES))
COMMON_OBJ_FILES := $(notdir $(COMMON_OBJ_FILES))
COMMON_OBJ_FILES := $(addprefix $(COMMON_OBJDIR)/, $(COMMON_OBJ_FILES))

# External libraries
# The "-lstdc++fs" one is needed because,as it turns out, some compilers still require it,
#	event the <filesystem> module has transitioned out of the experimental phase at c++17
LIB_FLAGS = -lpthread -lstdc++fs

# Name of program
TARGET =   ep_client

all: $(TARGET)

all_central: $(TARGET)
	mv $(TARGET) ..


$(TARGET): $(OBJ_FILES)
	$(MAKE) -C ../common
	$(GCC) -I $(INCLUDE_PATH) -I $(COMMON_INCLUDE_PATH) $^ $(COMMON_OBJ_FILES) -o $@ $(LIB_FLAGS)


$(OBJ_FILES): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp | $(OBJDIR)
	$(GCC) $(CPP_FLAGS) -I $(INCLUDE_PATH) -I $(COMMON_INCLUDE_PATH) -c $^ -o $@


$(OBJDIR):
	mkdir $@

clean:
	rm -rf ./$(TARGET)
	rm -rf bin

clean_central:
	rm -rf bin
	rm -rf $(COMMON_OBJDIR)
SRC_DIR=imgui
LIB_DIR=lib
VENDOR_DIR=..
LIB_NAME=imgui.a

SOURCES=$(wildcard $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:%.cpp=%.o)

# TODO: Don't _always_ use -g, etc.; release vs dev/debug
CXXFLAGS=-I$(SRC_DIR) -I$(VENDOR_DIR)/GLFW/include -I$(VENDOR_DIR)/GLEW/include -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DGLEW_NO_GLU -std=c++17 -g

CC=g++

.PHONY: static clean

static: $(OBJECTS)
	ar rcs $(LIB_DIR)/$(LIB_NAME) $(OBJECTS)

clean:
	rm -f $(OBJECTS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<
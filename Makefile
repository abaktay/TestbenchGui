# Compiler and flags
CXX = g++
CXXFLAGS = -fPIE -Wall -Iinc -Iexternal/imgui -Iexternal/imgui/backends `pkg-config --cflags glfw3` -DIMGUI_IMPL_OPENGL_LOADER_GLAD -std=c++23 -O2
LDFLAGS = `pkg-config --libs glfw3` -lGL

# Directories
SRC_DIR = src
BUILD_DIR = build
IMGUI_DIR = external/imgui

# Project source files
PROJECT_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
PROJECT_OBJS = $(patsubst $(SRC_DIR)/%.cpp, %.o, $(PROJECT_SOURCES))

# ImGui source files
IMGUI_CPP = imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp \
            backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl3.cpp
IMGUI_OBJS = $(IMGUI_CPP:.cpp=.o)

# Build object paths
PROJECT_BUILD_OBJS = $(addprefix $(BUILD_DIR)/, $(PROJECT_OBJS))
IMGUI_BUILD_OBJS = $(addprefix $(BUILD_DIR)/$(IMGUI_DIR)/, $(IMGUI_OBJS))
ALL_BUILD_OBJS = $(PROJECT_BUILD_OBJS) $(IMGUI_BUILD_OBJS)

# Final binary
TARGET = testbench

# Default target
all: $(TARGET)

# Link target
$(TARGET): $(ALL_BUILD_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Generic build rule for src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Generic build rule for ImGui
$(BUILD_DIR)/$(IMGUI_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Static library (optional)
libimgui.a: $(IMGUI_BUILD_OBJS)
	ar rcs $@ $^

$(TARGET)_static: $(PROJECT_BUILD_OBJS) libimgui.a
	$(CXX) -o $(TARGET) $^ $(LDFLAGS)

# Debug build
debug: CXXFLAGS += -g -DDEBUG -O0
debug: $(TARGET)

# Clean rules
clean:
	rm -rf $(BUILD_DIR) $(TARGET) libimgui.a

# Phony targets
.PHONY: all clean debug

# Include auto-generated dependency files
-include $(ALL_BUILD_OBJS:.o=.d)

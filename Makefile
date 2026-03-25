
APP_NAME     := Raymarcher
APP_ICON     := Raymarcher

COMPILER     := g++
INCLUDE_DIR  := Includes
SOURCE_DIR   := Sources
ASSEMBLY_DIR := Assemblies
BINARY_DIR   := Binaries
OBJECT_DIR   := Objects

SHADER_DIR   := Shaders
MAP_DIR      := Map

RELEASE_DIR  := Release

TARGET       := $(BINARY_DIR)/$(APP_NAME)
PCH          := $(INCLUDE_DIR)/pch.h

PKGS     := glfw3 glew
CXXFLAGS := -O0 -Wall -std=c++17 \
            $(shell pkg-config --cflags $(PKGS)) -I$(INCLUDE_DIR)
LDFLAGS  := $(shell pkg-config --libs $(PKGS)) -lopengl32

SOURCES = \
	Camera.cpp \
	Globals.cpp \
	Input.cpp \
	Main.cpp \
	Memory.cpp \
	Shader.cpp \
	UI.cpp \
	Window.cpp

OBJECTS       := $(SOURCES:%.cpp=$(OBJECT_DIR)/%.o)
#OBJECTS       += $(OBJECT_DIR)/$(APP_ICON).o
ASM_FILES     := $(SOURCES:%.cpp=$(ASSEMBLY_DIR)/%.s)
DEPENDENCIES  := $(OBJECTS:.o=.d)

##############################################################
all: $(TARGET)
assembly: $(ASM_FILES)
##############################################################

##############################################################
$(OBJECT_DIR)/$(APP_ICON).o: $(APP_ICON).rc | $(OBJECT_DIR)
	@echo Compiling $<...
	@windres $< -O coff -o $@
##############################################################

##############################################################
MKDIR = if not exist $1 mkdir $1
$(OBJECT_DIR) $(ASSEMBLY_DIR) $(BINARY_DIR):
	@$(call MKDIR,$(subst /,\,$@))
##############################################################

##############################################################
$(TARGET): $(PCH).gch $(OBJECTS) | $(BINARY_DIR)
	@echo Linking...
	@$(COMPILER) $(OBJECTS) -o $@ $(LDFLAGS)

$(PCH).gch: $(PCH)
	@echo Generating precompiled header...
	@$(COMPILER) $(CXXFLAGS) -x c++-header $< -o $@

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(PCH).gch | $(OBJECT_DIR)
	@echo Compiling $<...
	@$(COMPILER) $(CXXFLAGS) -include $(PCH) -c $< -o $@ -MMD -MP
##############################################################

##############################################################
$(ASSEMBLY_DIR)/%.s: $(SOURCE_DIR)/%.cpp $(PCH).gch | $(ASSEMBLY_DIR)
	@echo Generating assembly for $<...
	@$(COMPILER) $(CXXFLAGS) -include $(PCH) -S $< -o $@
##############################################################

##############################################################
-include $(DEPENDENCIES)
##############################################################

.PHONY: clean

clean:
	@echo Cleaning up...
	@if exist $(subst /,\,$(OBJECT_DIR)) rmdir /s /q $(subst /,\,$(OBJECT_DIR))
	@if exist $(subst /,\,$(ASSEMBLY_DIR)) rmdir /s /q $(subst /,\,$(ASSEMBLY_DIR))
	@if exist $(subst /,\,$(RELEASE_DIR)) rmdir /s /q $(subst /,\,$(RELEASE_DIR))
	@if exist $(subst /,\,$(INCLUDE_DIR))\pch.h.gch del /f /q $(subst /,\,$(INCLUDE_DIR))\pch.h.gch
	@if exist $(subst /,\,$(BINARY_DIR))\$(APP_NAME).exe del /f /q $(subst /,\,$(BINARY_DIR))\$(APP_NAME).exe

.PHONY: release
release:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory all
	@mkdir $(subst /,\,$(RELEASE_DIR))
	@copy /Y $(subst /,\,$(TARGET).exe) $(subst /,\,$(RELEASE_DIR)) > nul
	@echo Copying Shaders...
	@xcopy /E /I /Y $(SHADER_DIR) $(subst /,\,$(RELEASE_DIR))\$(SHADER_DIR) > nul
	@echo Copying Map...
	@xcopy /S /I /Y $(MAP_DIR)\*.ogg $(subst /,\,$(RELEASE_DIR))\$(MAP_DIR)\ > nul
	@powershell -NoProfile -ExecutionPolicy Bypass -File ReleaseDLLs.ps1 \
				-ReleaseDir "$(subst /,\,$(RELEASE_DIR))" -AppName "$(APP_NAME)"
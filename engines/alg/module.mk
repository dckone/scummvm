MODULE := engines/alg

MODULE_OBJS := \
	metaengine.o \
	alg.o \
	graphics.o \
	game.o \
	game_johnnyrock.o \
	game_maddog.o \
	game_maddog2.o \
	scene.o \
	video.o

MODULE_DIRS += \
	engines/alg

# This module can be built as a plugin
ifeq ($(ENABLE_ALG), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

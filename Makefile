CC = g++ -Wall
OTHER = -L/usr/X11R6/lib
LIB = -lGL -lGLU -lXxf86vm -lm -l3ds
OUTPUT = fighter
OUTPUTDBG = fighter_dbg
SOURCE_MATH = ./Math/*.cpp \
         ./Math/Cameras/*.cpp \
         ./Math/Figures/*.cpp \
         ./Math/Tracking/*.cpp
SOURCE_GRPH = ./Graphics/*.cpp \
         ./Graphics/OGL/*.cpp \
         ./Graphics/OGL/Extensions/*.cpp \
         ./Graphics/OGL/Fonts/*.cpp \
         ./Graphics/OGL/Render/*.cpp \
         ./Graphics/OGL/Textures/*.cpp
SOURCE_END = ./Config.cpp \
         ./App\ Framework/*.cpp \
         ./App\ Framework/OGL/*.cpp \
         ./App\ Framework/Input/*.cpp \
         ./Models/*.cpp \
         ./Models/lib3dx/*.cpp \
         ./Physics/*.cpp \
         ./Physics/Colliders/*.cpp \
         ./Physics/Verlet/*.cpp \
         ./Source\ Files/*.cpp \
         ./Utils/*.cpp \
         ./World/*.cpp
CPPFLAGS = -DGLX_GLXEXT_PROTOTYPES

release:
	$(CC) -O2 ${SOURCE_MATH} ${SOURCE_GRPH} ${SOURCE_END} -o ${OUTPUT} ${CPPFLAGS} ${OTHER} ${LIB}

debug:
	$(CC) -ggdb ${SOURCE_MATH} ${SOURCE_GRPH} ${SOURCE_END} -o ${OUTPUTDBG} ${CPPFLAGS} ${OTHER} ${LIB}

all: release
	

clean:
	@echo Cleaning up...
	@rm fighter
	@rm fighter_dbg

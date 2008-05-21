CC = g++ -Wall
OTHER = -L/usr/X11R6/lib
LIB = -lGL -lGLU -lXxf86vm -lm -l3ds
OUTPUT = fighter
OUTPUTDBG = fighter_dbg
SOURCE = ./Config.cpp \
         ./App\ Framework/*.cpp \
         ./App\ Framework/GL/*.cpp \
         ./App\ Framework/Input/*.cpp \
         ./GLExtensions/*.cpp \
         ./Math/*.cpp \
         ./Math/Cameras/*.cpp \
         ./Models/*.cpp \
         ./Models/lib3dx/*.cpp \
         ./OpenGL/*.cpp \
         ./OpenGL/Fonts/*.cpp \
         ./OpenGL/Textures/*.cpp \
         ./Physics/*.cpp \
         ./Source\ Files/*.cpp \
         ./Utils/*.cpp \
         ./World/*.cpp
CPPFLAGS = -DGLX_GLXEXT_PROTOTYPES

all:
	$(CC) ${SOURCE} -o ${OUTPUT} ${CPPFLAGS} ${OTHER} ${LIB}

release:
	$(CC) -O2 ${SOURCE} -o ${OUTPUT} ${CPPFLAGS} ${OTHER} ${LIB}

debug:
	$(CC) -ggdb ${SOURCE} -o ${OUTPUTDBG} ${CPPFLAGS} ${OTHER} ${LIB}

clean:
	@echo Cleaning up...
	@rm fighter
	@rm fighter_dbg
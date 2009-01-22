LIBS += -lGLEW \
        -lGLU \
        -lGL \
        -l3ds \
        -lXxf86vm \
        -lX11 \
        -llua5.1 \
        -lm

SRC = $(wildcard *.cpp) \
      $(wildcard */*.cpp) \
      $(wildcard */*/*.cpp) \
      $(wildcard */*/*/*.cpp) \
      $(wildcard */*/*/*/*.cpp)

CXX          = g++
CXXFLAGS     += -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-result $(INCLUDE_DIRS) $(INCLUDE_FILES) # -DGLX_GLXEXT_PROTOTYPES
LDFLAGS      +=

CXXFLAGS_RLS = $(CXXFLAGS) -s -O3 -DNDEBUG
LDFLAGS_RLS  = $(LDFLAGS)
OBJ_DIR_RLS  = obj/rls
OBJ_RLS      = $(patsubst %.cpp,$(OBJ_DIR_RLS)/%.o, $(SRC))
BIN_RLS      = fighter

CXXFLAGS_DBG = $(CXXFLAGS) -g -rdynamic -DDEBUG
LDFLAGS_DBG  = $(LDFLAGS) -rdynamic
OBJ_DIR_DBG  = obj/dbg
OBJ_DBG      = $(patsubst %.cpp,$(OBJ_DIR_DBG)/%.o, $(SRC))
BIN_DBG      = fighter_dbg

all: release

.PHONY : all clean release debug cleanrelease cleandebug

clean: cleanrelease cleandebug

cleanrelease:
	rm -rf $(OBJ_DIR_RLS)

cleandebug:
	rm -rf $(OBJ_DIR_DBG)

release: $(BIN_RLS)

debug: $(BIN_DBG)

$(BIN_RLS): $(OBJ_RLS)
	$(CXX) $(LDFLAGS_RLS) -o $@ $(OBJ_RLS) $(LIBS)

$(BIN_DBG): $(OBJ_DBG)
	$(CXX) $(LDFLAGS_DBG) -o $@ $(OBJ_DBG) $(LIBS)

$(OBJ_DIR_RLS)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_RLS) $< -c -o $@

$(OBJ_DIR_DBG)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_DBG) $< -c -o $@

$(OBJ_DIR_RLS)/%.d: %.cpp
	@set -e; \
		mkdir -p $(dir $@); \
		$(CXX) -MM $(CXXFLAGS_RLS) $< | sed 's,.*\.o[ :]*,$(OBJ_DIR_DBG)/$*.o $(OBJ_DIR_DBG)/$*.d : ,' > $@

$(OBJ_DIR_DBG)/%.d: %.cpp
	@set -e; \
		mkdir -p $(dir $@); \
		$(CXX) -MM $(CXXFLAGS_DBG) $< | sed 's,.*\.o[ :]*,$(OBJ_DIR_DBG)/$*.o $(OBJ_DIR_DBG)/$*.d : ,' > $@

#
# make deps if not cleaning
#
MAKE_DEPS=1
ifeq "$(MAKECMDGOALS)" "clean"
    MAKE_DEPS=0
endif
ifeq "$(MAKECMDGOALS)" "cleanrelease"
    MAKE_DEPS=0
endif
ifeq "$(MAKECMDGOALS)" "cleandebug"
    MAKE_DEPS=0
endif
ifeq "$(MAKE_DEPS)" "1"
    -include $(OBJ_RLS:.o=.d)
    -include $(OBJ_DBG:.o=.d)
endif



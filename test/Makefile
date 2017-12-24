NAME := DSDisplayTest

CC := gcc
CFLAGS := -O0 -g3 -Wall -std=c11

CXX := g++
CXXFLAGS := -O0 -g3 -Wall -std=c++1y

LD := g++

# Specify Linux include paths with -I directives here
IFLAGS := -Ihost

# Specify Linux libs with -l directives here
LDFLAGS := -pthread

# Assign executable name
EXEC := $(NAME)

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach dir,$(wildcard $1*),$(call rwildcard,$(dir)/,$2))

# Recursively find all C source files
SRC_C := $(call rwildcard,src/,*.c) $(call rwildcard,host/,*.c)

# Recursively find all C++ source files
SRC_CXX := $(call rwildcard,src/,*.cpp) $(call rwildcard,host/,*.cpp)

# Create raw list of object files
C_OBJ := $(SRC_C:.c=.o)
CXX_OBJ := $(SRC_CXX:.cpp=.o)

# Create list of object files
OBJDIR := build
C_OBJ := $(addprefix $(OBJDIR)/,$(C_OBJ))
CXX_OBJ := $(addprefix $(OBJDIR)/,$(CXX_OBJ))

.PHONY: all
all: $(OBJDIR)/$(EXEC)

# Define a string comparison function: "String EQual"
seq = $(and $(findstring $1,$2),$(findstring $2,$1))

# Define function for determining if given target is within a list of targets
# (If $1 is an element of $2)
# Returns a list whose size equals the number of occurences of $1 in $2
targetelem = $(foreach goal,$2, \
$(if $(call seq, $1 , $(goal) ),found,))

# Determine if any of the clean targets are about to be built
# If 'clean' won't be built
ifeq (,$(strip $(call targetelem,clean,$(MAKECMDGOALS))))

# Regenerate the dependencies
-include $(C_OBJ:.o=.d) $(CXX_OBJ:.o=.d)

endif

$(OBJDIR)/$(EXEC): $(C_OBJ) $(CXX_OBJ)
	@mkdir -p $(@D)
	@echo Linking $@
ifdef VERBOSE
	$(LD) -o $@ $(C_OBJ) $(CXX_OBJ) $(LDFLAGS)
else
	@$(LD) -o $@ $(C_OBJ) $(CXX_OBJ) $(LDFLAGS)
endif

# Pattern rule for building object file from C source
# The -MMD flag generates .d files to track changes in header files included in
# the source.
$(C_OBJ): $(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo Building C object $@
ifdef VERBOSE
	$(CC) $(CFLAGS) $(IFLAGS) -MMD -c -o $@ $<
else
	@$(CC) $(CFLAGS) $(IFLAGS) -MMD -c -o $@ $<
endif

# Pattern rule for building object file from C++ source
# The -MMD flag generates .d files to track changes in header files included in
# the source.
$(CXX_OBJ): $(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@echo Building CXX object $@
ifdef VERBOSE
	$(CXX) $(CXXFLAGS) $(IFLAGS) -MMD -c -o $@ $<
else
	@$(CXX) $(CXXFLAGS) $(IFLAGS) -MMD -c -o $@ $<
endif

.PHONY: clean
clean:
	@echo Removing object files
ifdef VERBOSE
	-$(RM) -r $(OBJDIR)/src $(OBJDIR)/host
	-$(RM) $(OBJDIR)/$(EXEC)
else
	-@$(RM) -r $(OBJDIR)/src $(OBJDIR)/host
	-@$(RM) $(OBJDIR)/$(EXEC)
endif

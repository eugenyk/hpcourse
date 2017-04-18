.PHONY: all clean

# Based on https://github.com/yeputons/project-templates

TARGETS=main
SRCS_main=$(wildcard $(SRCDIR)/*.cpp)
OBJDIR=.obj
SRCDIR=src
INCDIR=inc

all: $(TARGETS)
main: $(SRCS_main:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

CXX=g++
CXXFLAGS=-pedantic -Wall -Wshadow -Wextra -Werror -std=c++11 -I$(INCDIR) -pthread
LDFLAGS=-pthread
LDLIBS=

$(TARGETS):
	$(CXX) -o $@ $(LDFLAGS) $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(TARGETS) $(OBJDIR)

include $(wildcard $(OBJDIR)/*.d)

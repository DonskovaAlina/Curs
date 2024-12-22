Compiler = g++
RELEASE_FLAGS = -O2 -Wall -DNDEBUG
DEBUG_FLAGS   = -g -O0 -Wall
RELEASE_EXEC  = client
DEBUG_EXEC    = client-dbg
TEST_EXEC     = test_runner
SOURCES       = main.cpp client.cpp interface.cpp
TEST_SOURCES  = test.cpp client.cpp interface.cpp
LIBARY        = -lboost_program_options -lcryptopp
TEST_LIBS     = -lUnitTest++

.PHONY: all debug release clean test

all: debug release test

debug: $(DEBUG_EXEC)

$(DEBUG_EXEC): $(SOURCES)
	$(Compiler) $(DEBUG_FLAGS) $(SOURCES) -o $(DEBUG_EXEC) $(LIBARY)

release: $(RELEASE_EXEC)

$(RELEASE_EXEC): $(SOURCES)
	$(Compiler) $(RELEASE_FLAGS) $(SOURCES) -o $(RELEASE_EXEC) $(LIBARY)

test: $(TEST_EXEC)
	./$(TEST_EXEC)

$(TEST_EXEC): $(TEST_SOURCES)
	$(Compiler) $(DEBUG_FLAGS) $(TEST_SOURCES) -o $(TEST_EXEC) $(LIBARY) $(TEST_LIBS)

clean:
	rm -f $(RELEASE_EXEC) $(DEBUG_EXEC) $(TEST_EXEC)

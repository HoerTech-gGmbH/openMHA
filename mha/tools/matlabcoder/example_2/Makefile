# The generated sources we need to compile.
SOURCES = example_2.c example_2_emxAPI.c example_2_emxutil.c

# The name of the object files generated from the above sources
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))

# Rule how to compile the user library from the object files
# Replace .so with the appropriate suffix for the targeted operating
# system: .so for Linux, .dll for Windows, .dylib for macOS
example_2.so: $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

# Rule for compilation of source files
%.o: %.c
	$(CC) -c -o $@ $<

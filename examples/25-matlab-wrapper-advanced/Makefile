# Define the dependencies of example_25.so
# The rule for the compilation of the .o files is provided by Make
# as an implicit rule
example_25.so: example_25.o example_25_emxAPI.o example_25_emxutil.o
	$(CC) -shared -o $@ $^

# Define the dependencies of example_28.so
# The rule for the compilation of the .o files is provided by Make
# as an implicit rule
example_28.so: example_28.o example_28_emxAPI.o example_28_emxutil.o
	$(CC) -shared -o $@ $^

# Define the dependencies of example_27.so
# The rule for the compilation of the .o files is provided by Make
# as an implicit rule
example_27.so: example_27.o example_27_emxAPI.o example_27_emxutil.o
	$(CC) -shared -o $@ $^

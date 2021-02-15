- ./mha/plugins direcotry has a file matlabcoder_example0.cpp. 
This should be general to all wave domain input with a function named process.
- The directory also contains a Makefile.
- In this directory, C code can be generated using example_0_script.m via Matlab 
(Matlab coder is necessary).
- This generates a subdirectory codegen/lib/process/ in ./mha/tools/matlabcoder/example_0/.
- Though, only some files are necessary, 
one can copy all the files in the process folder to mha/plugins/matlabcoder_example0.
- This directory already contains the required generated code but for testing, 
this can be overwritten. In the future, it can be automated.
- Use make install to compile and install mha.
- Run make test to test the matlab generated plugin 
(Test in script .../mhatest/test_matlabcoder_example0.m).
- If test passes, the output in matlab and generated C code is identical.

LDLIBS=NAL-NL2.lib
CXXFLAGS=-std=c++17 -ggdb --static
nalnl2wrapper: nalnl2wrapper.cpp parser.cpp parser.hh nalnl2wrapper.hh NAL-NL2.lib
	$(CXX) $(CXXFLAGS) -o $@ nalnl2wrapper.cpp parser.cpp $(LDLIBS)

clean:
	rm -f nalnl2wrapper

test: test.cpp parser.cpp parser.hh
	$(CXX) $(CXXFLAGS) -o $@ test.cpp parser.cpp

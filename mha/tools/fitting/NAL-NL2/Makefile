LDLIBS=NAL-NL2.lib
CXXFLAGS=-std=c++17 -ggdb --static
nalnl2wrapper: nalnl2wrapper.cpp parser.cpp parser.hh nalnl2wrapper.hh NAL-NL2.lib
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ nalnl2wrapper.cpp parser.cpp $(LDFLAGS) $(LDLIBS)

clean:
	rm -f nalnl2wrapper

test: test.cpp parser.cpp parser.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ test.cpp parser.cpp $(LDFLAGS) $(LDLIBS)

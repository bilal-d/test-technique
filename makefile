CXX := c++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wfatal-errors

.PHONY: all
all: json_validator.prg test_char_reader.prg

json_validator.prg: json_validator.obj
	$(CXX) $(CXXFLAGS) -o $@ $+

test_char_reader.prg: test_char_reader.obj
	$(CXX) $(CXXFLAGS) -o $@ $+

%.obj: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f json_validator.prg
	rm -f json_validator.obj
	rm -f test_char_reader.prg
	rm -f test_char_reader.obj

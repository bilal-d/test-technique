CXX := c++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wfatal-errors

.PHONY: all
all: json_validator.prg

json_validator.prg: json_validator.obj
	$(CXX) $(CXXFLAGS) -o $@ $+

%.obj: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f json_validator.prg
	rm -f json_validator.obj

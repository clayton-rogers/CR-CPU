CXXFLAGS=-Wall -Wextra -Wpedantic -Wno-psabi -std=c++17
export CXXFLAGS # export to each of the sub-makes

CR-LIB=cr-cc-lib/cr-cc-lib.a
CR-CC=cr-cc/cr-cc.a
CR-TEST=cr-cc-test/cr-test.a

all: test cc

test: $(CR-TEST) $(CR-LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ cr-cc-test/*.o $(CR-LIB)

cc: $(CR-CC) $(CR-LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(CR-LIB) :
	cd cr-cc-lib && $(MAKE)

$(CR-CC) :
	cd cr-cc && $(MAKE)

$(CR-TEST) :
	cd cr-cc-test && $(MAKE)

clean:
	$(RM) test cc
	cd cr-cc-lib && $(MAKE) clean
	cd cr-cc && $(MAKE) clean
	cd cr-cc-test && $(MAKE) clean

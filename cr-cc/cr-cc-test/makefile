CXXFLAGS=-Wall -Wextra -Wno-psabi -std=c++17 -I../cr-cc-lib

SRCS=$(shell find . -name "*.cpp")
OBJS=$(subst .cpp,.o,$(SRCS))

cr-test.a: $(OBJS)
	ar crvs $@ $(OBJS)

clean:
	$(RM) cr-test.a
	$(RM) $(OBJS)

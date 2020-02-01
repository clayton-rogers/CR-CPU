CXXFLAGS=-Wall -Wextra -Wno-psabi -std=c++17 -I../cr-cc-lib

SRCS=$(shell find . -name "*.cpp")
OBJS=$(subst .cpp,.o,$(SRCS))

cr-cc.a: $(OBJS)
	ar crvs $@ $(OBJS)

clean:
	$(RM) cr-cc.a
	$(RM) $(OBJS)

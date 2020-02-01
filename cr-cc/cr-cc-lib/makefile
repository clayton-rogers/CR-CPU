# no extra CXXFLAGS required

SRCS=$(shell find . -name "*.cpp")
OBJS=$(subst .cpp,.o,$(SRCS))

cr-cc-lib.a: $(OBJS)
	ar crvs $@ $(OBJS)

clean:
	$(RM) cr-cc-lib.a
	$(RM) $(OBJS)

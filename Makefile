CXXFLAGS += -O3 -std=c++17 -Wall -Wpedantic
ifdef SYSTEMROOT
    CXXFLAGS += -lpthread
else
    CXXFLAGS += -pthread
endif
ifeq ($(SSE4),1)
    CXXFLAGS += -msse4.2
endif
SRCS := $(wildcard omp/*.cpp)
OBJS := ${SRCS:.cpp=.o}

all: lib/libompeval.a test

lib:
	mkdir lib

lib/libompeval.a: $(OBJS) | lib
	ar rcs $@ $^

test: test.cpp benchmark.cpp lib/libompeval.a
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) test test.exe lib/libompeval.a $(OBJS)
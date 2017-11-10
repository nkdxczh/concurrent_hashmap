CXXFILES = hashset
CXX	 = g++
CXXFLAGS = -std=c++11 -O3
LDFLAGS	 = -lm
TARGET   = hashset

.PHONY: all clean test

all: $(TARGET)

%:%.cc
	$(CXX) $(CXXFLAGS) $< -ltbb -o $@ $(LDFLAGS)

clean:
	$(RM) $(TARGET)

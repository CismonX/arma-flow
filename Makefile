SOURCES     = $(wildcard src/*.cpp)
OBJECTS     = $(SOURCES:%.cpp=%.o)
APPLICATION = arma-flow
CXXFLAGS    = -Wall -c -O2 -std=c++17
LDFLAGS     = -larmadillo -ljanus -lstdc++fs

all:            ${OBJECTS} ${APPLICATION}

${APPLICATION}: ${OBJECTS}
	${CXX} -o $@ ${OBJECTS} ${LDFLAGS}

${OBJECTS}:
	${CXX} ${CXXFLAGS} -o $@ ${@:%.o=%.cpp}

clean:
	rm -f ${APPLICATION} ${OBJECTS}

PDF := latex/refman.pdf
SRCS := $(wildcard ../*.cpp ../*.h)

all: ${PDF}
	atril ${PDF}

${PDF}: Doxyfile $(SRCS)
	doxygen
	cd ${@D} && make pdf


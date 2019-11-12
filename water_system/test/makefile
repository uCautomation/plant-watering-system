GTEST_REPO_DIR := googletest
GTEST_DIR := googletest/googletest

#TEST_SRCS := src/wssm-test.cc
#TEST_BIN  := ./wssm-test

SRCS := ../WateringSystemSM.cc

CXXFLAGS := -std=c++11 -isystem ${GTEST_DIR}/include -pthread
CXXFLAGS += -I.. -Imocks -I$(GTEST_DIR)

MOCKS_SRCS := $(wildcard mocks/*_mock.cpp)
TEST_SRCS  := $(wildcard src/test_*.cpp)

# UT_SRCS    := $(wildcard ../*.cpp)
UT_SRCS    := $(wildcard ../WaterSystem*.cpp) ../LcdGlyphs.cpp

.PHONY: test

run_tests: test
	./$<

test: $(MOCKS_SRCS) $(TEST_SRCS) $(UT_SRCS) $(GTEST_REPO_DIR)/build/lib/libgtest.a
	$(CXX) $(CXXFLAGS) ${GTEST_DIR}/src/gtest_main.cc $^ -o $@

$(GTEST_REPO_DIR)/build/lib/libgtest.a: | $(GTEST_REPO_DIR)/build
	cd $(dir $(@D)) && cmake .. && make

$(GTEST_REPO_DIR)/build/lib/libgmock.a: | $(GTEST_REPO_DIR)/build/lib/libgtest.a

$(GTEST_REPO_DIR)/build: $(GTEST_DIR)
	@mkdir $@

$(GTEST_DIR):
	git clone --depth 1 https://github.com/google/googletest.git
	cd googletest && git show -q --oneline

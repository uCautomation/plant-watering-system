GTEST_DIR := googletest

$(GTEST_DIR)/build/lib/libgtest.a: | $(GTEST_DIR)/build
	cd $(dir $(@D)) && cmake .. && make

$(GTEST_DIR)/build/lib/libgmock.a: | $(GTEST_DIR)/build/lib/libgtest.a

$(GTEST_DIR)/build: $(GTEST_DIR)
	@mkdir $@

$(GTEST_DIR):
	git clone --depth 1 https://github.com/google/googletest.git

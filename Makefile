TARGET := gen-countr-zero-c
TESTBITS := 8 16 32 64

$(TARGET): main.c
	$(CC) -o $@ $^

test/acutest.h:
	curl -Lo $@ https://github.com/mity/acutest/raw/31751b4089c93b46a9fd8a8183a695f772de66de/include/acutest.h

define GEN_TEST_MENU
build/test-$1bit: build/countr_zero_uint$1_t.c test/test_uint$1_t.c
	$$(CC) -g -o $$@ $$^

build/countr_zero_uint$1_t.c: $$(TARGET)
	@test -d build || mkdir build
	{ \
		echo '#include <stdint.h>'; \
		echo '#include <stddef.h>'; \
		echo ''; \
		./$$(TARGET) $1; \
		./$$(TARGET) --omit-support-zero $1; \
	} > $$@

.PHONY: run-test-$1bit
run-test-$1bit: build/test-$1bit
	./$$<
endef

$(foreach i,$(TESTBITS),$(eval $(call GEN_TEST_MENU,$i)))

.PHONY: test-all
test-all: $(TESTBITS:%=build/test-%bit)
	$(foreach i,$(TESTBITS),./build/test-$ibit;)

gen-testcases:
	$(foreach i,$(TESTBITS),./test/gen-testcase.sh $i;)

.PHONY: clean
clean:
	$(RM) $(TARGET)
	$(RM) -r build

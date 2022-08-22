#TODO: replace ID with your own IDS, for example: 123456789_123456789
SUBMITTERS := 315239673_209512698
COMPILER := g++
COMPILER_FLAGS := --std=c++11 -Wall -g
SRCS := Commands.cpp smash.cpp small_shell.cpp jobs.cpp signals.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
HDRS := Commands.h  small_shell.h jobs.h exceptions.h signals.h
TESTS_INPUTS := $(wildcard test_input*.txt)
TESTS_OUTPUTS := $(subst input,output,$(TESTS_INPUTS))
SMASH_BIN := smash
.PHONY: $(TESTS_OUTPUTS)
test: $(TESTS_OUTPUTS)

$(TESTS_OUTPUTS): $(SMASH_BIN)
$(TESTS_OUTPUTS): test_output%.txt: test_input%.txt test_expected_output%.txt
	./$(SMASH_BIN) < $(word 1, $^) > $@
	diff $@ $(word 2, $^)
	echo $(word 1, $^) ++PASSED++ 

$(SMASH_BIN): $(OBJS)
	$(COMPILER) $(COMPILER_FLAGS) $^ -o $@

$(OBJS): %.o: %.cpp
	$(COMPILER) $(COMPILER_FLAGS) -c $^

zip: $(SRCS) $(HDRS)
	zip $(SUBMITTERS).zip $^ submitters.txt Makefile

clean:
	rm -rf $(SMASH_BIN) $(OBJS) $(TESTS_OUTPUTS) 
	rm -rf $(SUBMITTERS).zip

include definitions.mk

.NOTPARALLEL:

$(patsubst %,%-subdir-unit-tests,$(MODULES)):
	$(MAKE) -C $(@:-subdir-unit-tests=) unit-tests

unit-tests: $(patsubst %,%-subdir-unit-tests,$(MODULES))

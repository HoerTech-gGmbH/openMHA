include definitions.mk

.NOTPARALLEL:

$(patsubst %,%-subdir-unit-tests,$(MODULES)):
	$(MAKE) -C $(@:-subdir-unit-tests=) unit-tests

unit-tests-internal: $(patsubst %,%-subdir-unit-tests,$(MODULES))

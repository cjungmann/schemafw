VERSION_PATCH_FILE=VERSION_PATCH.txt
VERSION_MAJOR_FILE=VERSION_MAJOR.txt
VERSION_MINOR_FILE=VERSION_MINOR.txt


VERSION_LINK_FLAGS = -Xlinker --defsym=__BUILD_DATE=$$(date +'%Y%m%d')
VERSION_LINK_FLAGS += -Xlinker --defsym=__VERSION_MAJOR=$$(cat $(VERSION_MAJOR_FILE))
VERSION_LINK_FLAGS += -Xlinker --defsym=__VERSION_MINOR=$$(cat $(VERSION_MINOR_FILE))
VERSION_LINK_FLAGS += -Xlinker --defsym=__VERSION_PATCH=$$(cat $(VERSION_PATCH_FILE))


VERSION_DEPS=$(VERSION_MAJOR_FILE) $(VERSION_MINOR_FILE) $(VERSION_PATCH_FILE)

# Use make rules to initialize missing version files:
$(VERSION_MAJOR_FILE) :
	@if [ ! -e $@ ]; then \
		echo 0 > $@ ; \
	fi

$(VERSION_MINOR_FILE) :
	@if [ ! -e $@ ]; then \
		echo 0 > $@ ; \
	fi

$(VERSION_PATCH_FILE) :
	@if [ ! -e $@ ]; then \
		echo 0 > $@ ; \
	fi


# Copy a zero to the names file
define zero_file
	@echo 0 > $(1)
endef

# Create a file with '0" only if the file does not exist:
# No longer necessary after adding rules to create the files:
define make_zero_build
	@if [ ! -e $(1) ]; then \
		echo 0 > $(1); \
   fi
endef

# Make sure all necessary version files exist.
define make_if_missing
	$(call make_zero_build,$(VERSION_PATCH_FILE))
	$(call make_zero_build,$(VERSION_MAJOR_FILE))
	$(call make_zero_build,$(VERSION_MINOR_FILE))
endef

# Prints the unformated version number.
# Use to confirm the version number before building.
define echo_full_version
	@echo $$(cat $(VERSION_MAJOR_FILE)).$$(cat $(VERSION_MINOR_FILE)).$$(cat $(VERSION_PATCH_FILE))
endef

# As function name implies, add one to the named file
define increment_file
	$(call make_if_missing)
	@echo $$(( $$(cat $(1)) + 1 )) > $(1)
endef



# Local Variables:
# mode: makefile-gmake
# End:


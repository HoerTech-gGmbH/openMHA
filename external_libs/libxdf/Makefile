# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2022 Hoerzentrum Oldenburg gGmbH
#
# openMHA is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# openMHA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License, version 3 for more details.
#
# You should have received a copy of the GNU Affero General Public License,
# version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

include ../../config.mk
include ../../magic.mk

TARGETS=libxdf.a

include ../../rules.mk

OBJECTS = \
	xdfwriter.o

libxdf.a: $(BUILD_DIR)/libxdf.a

$(BUILD_DIR)/libxdf.a: $(OBJECTS:%.o=$(BUILD_DIR)/%.o)
	ar rcs $@ $^

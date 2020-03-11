# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2020 HörTech gGmbH
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

# This Makefile serves only as example. The user is responsible that the lsl headers and lib can be found.

include ../../config.mk
receive_lsl: send_lsl.cpp
	$(CXX) send_lsl.cpp -std=$(CXXSTANDARD) -g -o send_lsl -llsl
# Local Variables:
# compile-command: "make"
# End:

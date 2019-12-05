# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2019 HörTech gGmbH
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


NEEDS_EIGEN = yes
include ../plugin.mk

# Eigen uses some functors that were deprecated with c++11 and will be
# removed from standard c++ with c++17. They seem to fix these currently
# in development branch, therefore, accept the deprecated usage for now:
$(PLUGIN_ARTIFACTS): CXXFLAGS += -Wno-error=deprecated-declarations -Wno-duplicated-branches
# Local Variables:
# compile-command: "make"
# End:

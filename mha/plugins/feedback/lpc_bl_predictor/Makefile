# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2016 2017 HörTech gGmbH
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
{
  "targets": [
    {
      "target_name": "mha-node",
        "sources": [ "fw_t_wrap.cxx", "mhafw_lib.cpp" ],
      "include_dirs": ["../../libmha/src"],
      "libraries": ["-L<(module_root_dir)/../../libmha/x86_64-Darwin-clang",
                   "-lopenmha"],
        "cflags_cc": ["-fexceptions"]
    }
  ]
}

# Local Variables:
# compile-command: "make"
# End:

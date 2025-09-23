@echo off


rem This file is part of the HörTech Open Master Hearing Aid (openMHA)
rem Copyright © 2017 2018 2019 2020 2021 HörTech gGmbH
rem Copyright © 2023 2024 2025 Hörzentrum Oldenburg gGmbH
rem 
rem openMHA is free software: you can redistribute it and/or modify
rem it under the terms of the GNU Affero General Public License as published by
rem the Free Software Foundation, version 3 of the License.
rem 
rem openMHA is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU Affero General Public License, version 3 for more details.
rem 
rem You should have received a copy of the GNU Affero General Public License, 
rem version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.


start qjackctl
cls
mha --interactive ?read:cfg/HSIM_live.cfg
pause

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

all: ../mfiles/mhactl_java.jar

JAVAC=/usr/lib/jvm/java-8-openjdk-amd64/bin/javac
JAVA_SOURCES = $(shell find . -name *.java)
CLASS_FILES = $(patsubst %.java, %.class, $(JAVA_SOURCES))

clean:
	rm -f $(CLASS_FILES)

%.class: %.java
	$(JAVAC) -target 1.5 -source 1.5 $<

../mfiles/mhactl_java.jar: $(CLASS_FILES)
	jar -cvf $@ $^

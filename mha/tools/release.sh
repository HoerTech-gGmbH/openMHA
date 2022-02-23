#!/bin/bash -e
# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2019 2020 2021 HörTech gGmbH
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

#Prompt the user to answer yes or no, repeat question on any other answer, exit when answer is no.

function ask_yes_no ()
{
    local ANSWER=""
    read ANSWER
    while [[ "x$ANSWER" != "xyes" ]] && [[ "x$ANSWER" != "xno" ]]; do
        echo "Please answer yes or no"
        read ANSWER
    done
    if [[ $ANSWER = "no" ]]; then
       echo "Exiting..."
       exit 1;
    fi
}

if [ -n "$(git status --porcelain)" ]; then
    echo "The git repository contains uncommitted changes, exiting."
    exit 1;
fi

dry_run=false
#Prevent user from accidentally calling script as it should only be called by make
if [[ "x$1" != "xopenMHA" ]]; then
    echo "ATTENTION: This script was not called by make. Do you want to perform a dry run?"
    echo "A dry run will modify your local git but not push anything to remotes."
    ask_yes_no
    dry_run=true
fi

#Terminate when branch is not the development branch.
BRANCH=$(git branch | grep '*' | cut -d" " -f2);
if [[ "$BRANCH" != "development" ]]; then
    echo "Suspicious branch: $BRANCH. Expected: development"
    exit 1
fi

#master will be fast-forwarded to the state of branch development

echo "Have you tested the live pre-release tests as described in"
echo "https://dev.openmha.org/w/releaseprotocol/, 'Release procedure' step 3, and"
echo "sections 'test_mhaioalsa.m and other automated live tests' and 'Run gain_live"
echo "example, dynamic compressor live example, localizer live example'? [yes|no]"
ask_yes_no;

echo "Was everything as expected? (refer to"
echo "https://dev.openmha.org/w/releaseprotocol/ for expected behaviour) [yes/no]"
ask_yes_no;

#Prompt for new version number and change version number in manual and code to new version.
#Our normal version nomenclature is MAJOR.MINOR.PATCH, anything else requires a user override
printf "Enter new version number (e.g. 1.2.3): "
read VER
if ! [[ $VER =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Warning: Version does not follow usual convention: $VER. Continue? [yes/no]"
    ask_yes_no
fi

MAJOR_OLD=`grep "define MHA_VERSION_MAJOR" mha/libmha/src/mha.hh | sed -E 's/.*[^0-9]([0-9]+).*$/\1/g'`
MINOR_OLD=`grep "define MHA_VERSION_MINOR" mha/libmha/src/mha.hh | sed -E 's/.*[^0-9]([0-9]+).*$/\1/g'`
POINT_OLD=`grep "define MHA_VERSION_RELEASE" mha/libmha/src/mha.hh | sed -E 's/.*[^0-9]([0-9]+).*$/\1/g'`

MAJOR_NEW=`echo $VER | cut -d"." -f1`
MINOR_NEW=`echo $VER | cut -d"." -f2`
POINT_NEW=`echo $VER | cut -d"." -f3`

VERSIONS="$MAJOR_OLD.$MINOR_OLD.$POINT_OLD $MAJOR_NEW.$MINOR_NEW.$POINT_NEW"
SORTED_VERSIONS=$(echo $VERSIONS | xargs -n1 | sort -V | xargs)
if [[ "$VERSIONS" != "$SORTED_VERSIONS" ]]; then
    echo "Warning: New version number $MAJOR_NEW.$MINOR_NEW.$POINT_NEW is not an increase. Continue? [yes/no]"
    ask_yes_no
fi

if [[ $MAJOR_OLD = $MAJOR_NEW && $MINOR_OLD = $MINOR_NEW && $POINT_OLD = $POINT_NEW ]]; then
    echo "Warning: Old version number and new version number are equal! $VER. Continue? [yes/no]"
    ask_yes_no
fi

sed -i "s/$MAJOR_OLD\\.$MINOR_OLD\\.$POINT_OLD/$VER/g" README.md
sed -i "s/^#define MHA_VERSION_MAJOR $MAJOR_OLD$/#define MHA_VERSION_MAJOR $MAJOR_NEW/g" mha/libmha/src/mha.hh
sed -i "s/MHA_VERSION_MINOR $MINOR_OLD/MHA_VERSION_MINOR $MINOR_NEW/g" mha/libmha/src/mha.hh
sed -i "s/MHA_VERSION_RELEASE $POINT_OLD/MHA_VERSION_RELEASE $POINT_NEW/g" mha/libmha/src/mha.hh
sed -i "s/$MAJOR_OLD\\.$MINOR_OLD\\.$POINT_OLD/$VER/g" mha/doc/openMHAdoxygen.sty
sed -i -re "s/2[0-9]{3}-[0-9]{2}-[0-9]{2}/$(date +%Y-%m-%d)/g" README.md
sed -i "s/$MAJOR_OLD\\.$MINOR_OLD\\.$POINT_OLD/$VER/g" mha/tools/packaging/exe/mha.nsi
sed -i "s/$MAJOR_OLD\\.$MINOR_OLD\\.$POINT_OLD/$VER/g" mha/tools/packaging/pkg/Makefile
sed -i "s/$MAJOR_OLD\\.$MINOR_OLD\\.$POINT_OLD/$VER/g" mha/tools/packaging/ports/Portfile
sed -i "s/$MAJOR_OLD\\.$MINOR_OLD\\.$POINT_OLD/$VER/g" version
#deb package version is extracted from mha.hh

git commit -a -m"Increase version number to $VER"
git clean -fdx . 2>/dev/null 1>/dev/null;
echo "Testing regeneration of documentation..."
# This will locally generate PDFs and pdf-$VER.zip.
./configure 1>/dev/null && make -j9 doc 1>/dev/null 2>/dev/null
printf "Documentation generated correctly? [yes/no]"
ask_yes_no;

git checkout master && git pull && git merge --ff-only $BRANCH && git commit --allow-empty -m"Prepare Release $VER"

echo "Last command: git checkout master && git pull && git merge --ff-only $BRANCH && git commit --allow-empty -m\"Prepare Release $VER\""
echo "Type yes when ready to proceed"
ask_yes_no

echo "Before pushing to our internal gitolite server, did you enable pushing to master there [yes/no]?"
ask_yes_no

echo "Push new release to internal server [yes/no]?"
if $dry_run
then
    echo "SKIPPING PUSH BECAUSE OF DRY RUN."
else
    ask_yes_no
    git push
fi

echo "Did you disable the push to master again [yes/no]?"
ask_yes_no

git tag -a v$VER -m"Release $VER"

echo "Merging master back into development..."
git checkout development
git merge master
echo "merged master back into development with git checkout development && git merge master."
echo "Type yes when ready to proceed"
ask_yes_no

echo "The new release should soon be built by Jenkins. When this finishes without"
echo "errors, then "
echo " 1) check the documents generated by the Jenkins Job again,"
echo " 2) check the windows, mac and linux installers on test machines,"
echo "When all of the above is done, push new source release to Github? [yes/no]"
if $dry_run
then
    echo "SKIPPING PUSH BECAUSE OF DRY RUN."
else
    ask_yes_no;
    git push git@github.com:HoerTech-gGmbH/openMHA.git master
    git push git@github.com:HoerTech-gGmbH/openMHA.git v$VER
    git push origin development
    git push origin v$VER
fi
echo "Now create a GitHub release and copy the release notes into the description field."
echo "Sign macOS and Windows installers according to the instructions at"
echo "https://dev.openmha.org/w/releaseprotocol/ and attach them to the GitHub release."
echo "Answer yes when finished."
ask_yes_no
echo "Now the Linux installers need to be published to our apt repository."
echo "Open Jenkins in webbrowser, navigate to the last openMHA master branch build,"
echo "select replay, then search in the Jenkinsfile for the comment containing"
echo '"Releases are not automatically" ... and follow the instructions in this comment.'
echo "Answer yes when finished."
ask_yes_no

if $dry_run
then
    echo "The dry run ends here, it cannot copy regenerated PDFs from Jenkins to gh-pages."
    echo "The dry run has not pushed but locally modified your branches and tags."
    echo "Use git reset and git tag in the affected branches with suitable arguments"
    echo "to undo these local effects, or delete this local working copy."
    echo "MAKE SURE NOT TO PUSH THESE CHANGES TO A REMOTE"
    exit 0
fi

# Remove locally generated pdf.zip created earlier
rm -f pdf-$VER.zip
# Get PDF documents from Jenkins and publish on openMHA website (=branch gh-pages)
echo "Download the generated PDFs from Jenkins for upload to github-pages as $PWD/pdf-$VER.zip Type 'yes' when you are done."
ask_yes_no
# Temporarily, we need a separate clone for changing branch gh-pages:
git clone --branch=gh-pages --single-branch git@github.com:HoerTech-gGmbH/openMHA GHPAGES
cd GHPAGES/docs
unzip ../../pdf-$VER.zip
git add *.pdf
git commit -m "Update PDF documents to version $VER"
if $dry_run
then 
    echo SKIPPING PUSH OF DOCUMENTS TO BRANCH GH-PAGES BECAUSE OF DRY RUN.
else
    git push
fi
cd ../..
rm -rf GHPAGES
echo "Check that the Zenodo webhook has correctly created a new entry at https://zenodo.org/record/4569575"
echo "Click on the DOI badge to copy the Markdown code and include the Zenodo DOI badge into the release."
echo "Add the supplemental information to the release. Type 'yes' when finished"
ask_yes_no

echo "After openMHA is now released, we need to create a new Mahalia image for"
echo "the PHL that contains the new openMHA packages and reflects the new openMHA"
echo "version number in its name. This process may begin only after the deb"
echo "packages have been successfully published in our apt repository."
echo "Type 'yes' when ready to begin."
ask_yes_no
git clone git@github.com:mahalia-dependencies/mahalia-dependencies
pushd mahalia-dependencies
echo "$VER"-r0 >version
git add version
git commit -m "Release Mahalia $VER"-r0
git push
popd
rm -rf mahalia-dependencies

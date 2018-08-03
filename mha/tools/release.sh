#!/usr/bin/env bash

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

#Ask for user input when branch name is suggests neither a release branch
#nor the development branch. Our workflow currently prescribes a squash merge
#from development or a release branch in preparation for a release. If the branch
#does not match either, we ask for a user override.
BRANCH=$(git branch | grep '*' | cut -d" " -f2);
if  ! [[ "$BRANCH" =~ *release* ]] & ! [[ "$BRANCH" = "development" ]]; then
    echo "Suspicious branch: $BRANCH is neither a development or release branch. Continue? [yes/no];"
    ask_yes_no;
fi


#$BRANCH will be squash merged into master
echo "Releasing from branch $BRANCH..."

echo "Unit tests and system tests ran succesfully, now testing live examples."
echo "When prompted, please start a JACK server with the settings needed by the live example."

echo "Please start a JACK server for 00-gain and run the live configuration!"
echo "Was everything as expected? [yes/no]"
ask_yes_no;

echo "Please start a JACK server for 01-dynamic-compression and run the live configuration!"
echo "Was everything as expected? [yes/no]"
ask_yes_no;

echo "Please start a JACK server for 09-localizer-steering-beamformer and run the live configuration!"
echo "Was everything as expected? [yes/no]"
ask_yes_no;


#Prompt for new version number and change version number in manual and code to new version.
#Our normal version nomenclature is MAJOR.MINOR.PATCH, anything else requires a user override
printf "Enter new version number (e.g. 1.2.3): "
read VER
if ! [[ $VER =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Warning: Version does not follow usual convention: $VER. Continue? [yes/no]"
    ask_yes_no
fi

MAJOR_OLD=`grep "define MHA_VERSION_MAJOR" mha/libmha/src/mha.h | sed -E 's/.*[^0-9]([0-9]+).*$/\1/g'`
MINOR_OLD=`grep "define MHA_VERSION_MINOR" mha/libmha/src/mha.h | sed -E 's/.*[^0-9]([0-9]+).*$/\1/g'`
POINT_OLD=`grep "define MHA_VERSION_RELEASE" mha/libmha/src/mha.h | sed -E 's/.*[^0-9]([0-9]+).*$/\1/g'`

MAJOR_NEW=`echo $VER | cut -d"." -f1`
MINOR_NEW=`echo $VER | cut -d"." -f2`
POINT_NEW=`echo $VER | cut -d"." -f3`

VERSIONS="$MAJOR_OLD.$MINOR_OLD.$POINT_OLD $MAJOR_NEW.$MINOR_NEW.$POINT_NEW"
SORTED_VERSIONS=$(echo $VERSIONS | xargs -n1 | sort -V | xargs)
if [[ "$VERSIONS" != "$SORTED_VERSIONS" ]]; then
    echo "Warning: New version number $MAJOR_NEW.$MINOR_NEW.$POINT_NEW is not an increase. Continue? [yes/no]"
    ask_yes_no
fi

if [[ $MAJOR_OLD==$MAJOR_NEW ]] && [[ $MINOR_OLD==$MINOR_NEW ]] && [[ $POINT_OLD == $POINT_NEW ]]; then
    echo "Warning: Old version number and new version number are equal! $VER. Continue? [yes/no]"
    ask_yes_no
fi

sed -i "s/$MAJOR_OLD\\.$MINOR_OLD\\.$POINT_OLD/$VER/g" README.md
sed -i "s/^#define MHA_VERSION_MAJOR $MAJOR_OLD$/#define MHA_VERSION_MAJOR $MAJOR_NEW/g" mha/libmha/src/mha.h
sed -i "s/MHA_VERSION_MINOR $MINOR_OLD/MHA_VERSION_MINOR $MINOR_NEW/g" mha/libmha/src/mha.h
sed -i "s/MHA_VERSION_RELEASE $POINT_OLD/MHA_VERSION_RELEASE $POINT_NEW/g" mha/libmha/src/mha.h
sed -i "s/$MAJOR_OLD\\.$MINOR_OLD\\.$POINT_OLD/$VER/g" mha/doc/openMHAdoxygen.sty
sed -i -re "s/2[0-9]{3}-[0-9]{2}-[0-9]{2}/$(date +%Y-%m-%d)/g" README.md
git commit -a -m"Increase version number to $VER"
git clean -fdx . 2>/dev/null 1>/dev/null;
echo "Regenerating documentation..."
./configure 1>/dev/null && yes | make -j5 doc 1>/dev/null 2>/dev/null
printf "Documentation generated correctly? [yes/no]"
ask_yes_no;
git commit *.pdf -m "Regenerate Documentation for release $VER"
git checkout master && git merge --squash $BRANCH && git commit -m"Release $VER"
echo "Do you want to create packages now? [yes/no]"
set -o pipefail
(ask_yes_no) | sed '/.*Exiting.*/d'
if  ! [[ $? ]]; then
    make deb
fi
echo "All tests complete. Push new release to internal server [yes/no]?"
ask_yes_no
git push
git tag -a v$VER -m"Release $VER"
git checkout development
git merge master
echo "Push new release to Github? [yes/no]"
ask_yes_no;
git push git@github.com:HoerTech-gGmbH/openMHA.git master
git push git@github.com:HoerTech-gGmbH/openMHA.git v$VER

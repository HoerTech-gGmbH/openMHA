#!/bin/bash -ex

# A shell script that determines if the external_libs branch needs an update
# and if yes, updates it

# We can get the current branch. Get it.
branch_name="$BRANCH_NAME"

# Ensure that branches $branch_name and external_libs_$branch_name exist
git checkout "$branch_name"
git pull
if ! git checkout external_libs_"$branch_name"
then       # no external_libs version of this branch
    exit 0 # nothing to do
fi
git pull

# If we are still here, then we are on branch $branch_name, and both branches,
# $branch_name and external_libs_$branch_name do exist.
# We need to determine if the external_libs directory (this is the directory
# that contains this script) has any content differences between these two.
extlibsdir=$(dirname "$0")
if ! git diff --exit-code --quiet external_libs_"$branch_name" "$branch_name" \
              -- "$extlibsdir"
then
    # There are differences, we need to fast-forward the external_libs branch
    git merge --ff-only "$branch_name"
    # And publish the new external_libs_$branch_name branch to the repository
    git push
fi

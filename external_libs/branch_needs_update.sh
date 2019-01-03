#!/bin/bash -e

# A shell script that determines if the external_libs branch needs an update

# if we cannot get the current branch name, exit with false (option -e exits)
git rev-parse --abbrev-ref HEAD

# We can get the current branch. Get it.
branch_name=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)

# Check if branch external_libs/$branch_name exists. exit with false if it not.
git rev-parse --verify --quiet external_libs/"$branch_name"

# If we are still here, then we are on branch $branch_name, and both branches,
# $branch_name and external_libs/$branch_name do exist.
# We need to determine if the external_libs directory (this is the directory
# that contains this script) has any content differences between these two.
extlibsdir=$(dirname "$0")
git diff --exit-code --quiet external_libs/"$branch_name" "$branch_name" \
         -- "$extlibsdir"

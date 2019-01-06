#!/bin/bash

maximum_cache_age_in_minutes=1
directory="$(dirname $0)"
git_commit_hash_cache_file="$directory"/git_commit_hash.txt

function ask_git_for_hash() {
    git log -1 --abbrev=12 --pretty="format:%h"
}
function ask_git_if_modified() {
    test -z "`git status --porcelain -uno`" || echo "-modified"
}
function ask_git() {
    echo $(ask_git_for_hash)$(ask_git_if_modified)
}
function write_commit_hash_into_cache_file() {
    echo $(ask_git) >"$git_commit_hash_cache_file"
}
function does_cache_file_exist() {
    test -f "$git_commit_hash_cache_file"
}
function is_cache_file_missing() {
    ! does_cache_file_exist
}
function is_cache_file_too_old() {
    # only useful result if file exists
    # then it returs success if the file is too old, failure if it is recent
    test $(find "$git_commit_hash_cache_file" \
		-mmin +$maximum_cache_age_in_minutes)
}
function do_we_have_to_ask_git() {
    is_cache_file_missing || is_cache_file_too_old
}

if do_we_have_to_ask_git
then write_commit_hash_into_cache_file
fi

cat "$git_commit_hash_cache_file"

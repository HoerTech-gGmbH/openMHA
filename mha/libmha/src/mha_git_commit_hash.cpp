#include "mha_git_commit_hash.hh"

#ifndef GITCOMMITHASH
#define GITCOMMITHASH "independent-plugin-build"
#endif

/// store git commit hash in every binary plgin to support reproducible research
const char* mha_git_commit_hash =
  "MHA_GIT_COMMIT_HASH=" GITCOMMITHASH;

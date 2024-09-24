#!/bin/bash

P_OPT=
[ -n "$MINGW_PREFIX" ] && P_OPT=--binary
P_DIR="$(dirname "$0")"

# Apply and commit each patch
for patch in "$P_DIR"/00*.patch; do
  # Apply the patch
  patch $P_OPT -p1 < "$patch" || exit
  # Extract commit message from patch file and remove "Subject: " prefix
  commit_message=$(sed -n '4s/^Subject: //p' "$patch")
  # Add changes to git
  git add -A
  # Commit changes with the extracted commit message
  git commit -m "$commit_message"
done

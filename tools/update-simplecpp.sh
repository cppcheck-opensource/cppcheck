#!/usr/bin/env bash

set -e

files=(simplecpp.cpp simplecpp.h)
simplecpp_url="git@github.com:danmar/simplecpp.git"
commit_message_format="simplecpp: bump to %s"
cppcheck_dir="$(dirname "$(realpath --no-symlinks "${BASH_SOURCE[@]}")")/.."

tag="$1"
simplecpp_dir="$2"

[ -z "$tag" ] && {
	>&2 echo "Usage: $0 <tag> [checked out simplecpp repo]"
	exit 1
}

# Clone simplecpp if checked out repo was not provided
[ -z "$simplecpp_dir" ] && {
	simplecpp_dir="$(mktemp -d)"
	git clone "$simplecpp_url" "$simplecpp_dir"
}

# Checkout tag
git -C "$simplecpp_dir" -c advice.detachedHead=false checkout "$tag"

# Make sure tag is on the master branch
git -C "$simplecpp_dir" merge-base --is-ancestor "$tag" master || {
	>&2 echo "Tag $tag is not on the master branch"
	exit 1
}

# Make sure we don't commit any other files
git -C "$cppcheck_dir" reset --mixed

# Copy and stage files
for file in "${files[@]}"; do
	source="$simplecpp_dir/$file"
	dest="$cppcheck_dir/externals/simplecpp/$file"
	cp "$source" "$dest"
	git -C "$cppcheck_dir" add "$dest"
done

# Create commit
commit_message="$(printf "$commit_message_format" "$tag")"
git -C "$cppcheck_dir" commit -m "$commit_message"

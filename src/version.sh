#! /bin/bash
#VERSION=_N_VERS_
if [[ -n "$VERSION" ]]; then
	echo $VERSION
else
	IS_GIT=`git rev-parse --is-inside-work-tree`
	if [[ "$IS_GIT" == "true" ]]; then
		BRANCH=`git branch | sed --quiet 's/* \(.*\)/\1/p'`
		VERSION=`git name-rev master --tags | awk '{sub("tags/", ""); print $2}'`
		if [[ "$BRANCH" == "master" ]]; then
			echo $VERSION
		else
			echo $VERSION~$BRANCH
		fi
	else
		while IFS= read -r line <&3; do
			if [[ "$line" =~ \*[[:space:]]([0-9\.]+),[[:space:]](.*) ]]; then
				echo "${BASH_REMATCH[1]}~broken"
				break
			fi
		done 3< `dirname $0`/../ChangeLog
	fi
fi

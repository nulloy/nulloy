#! /bin/bash

BASENAME=`basename $0`
TRY_HELP="Try \`$BASENAME --help' for more information"

echo_help()
{
	echo "Usage: $BASENAME [options]"
	echo '-i, --input       input changelog file'
	echo '-d, --deb         output changelog file in deb format'
	echo '-r, --rpm         output changelog file in rpm format'
	echo '-c, --commiter    comitter name and email'
	echo '-p, --package     package name'
	echo '-h, --help        print this message'
	echo
	echo "Version 0.1"
	echo 'Copyright (C) 2011 Sergey Vlasov <sergey@vlasov.me>'
}

args=`getopt -n$BASENAME -o i:d:r:c:p:h --longoptions="input: deb: rpm: commiter: package: help" -- "$@"`
if test $? != 0; then
	echo $TRY_HELP
	exit 1
fi

eval set -- $args
while [[ $# -gt 0 ]]; do
	if [[ $1 == -i ]] || [[ $1 == --input ]]; then
		shift; INPUT=$1
	elif [[ $1 == -d ]] || [[ $1 == --deb ]]; then
		shift; OUT_DEB=$1
	elif [[ $1 == -r ]] || [[ $1 == --rpm ]]; then
		shift; OUT_RPM=$1
	elif [[ $1 == -c ]] || [[ $1 == --commiter ]]; then
		shift; COMMITER=$1
	elif [[ $1 == -p ]] || [[ $1 == --package ]]; then
		shift; PACKAGE=$1
	elif [[ $1 == -h ]]; then
		echo_help; exit 0
	elif [[ $1 == -- ]]; then
		shift
		if [[ $# -ne 0 ]]; then
			echo "$BASENAME: invalid arguments"
			echo $TRY_HELP; exit 1
		fi
		break
	fi
	shift
done

if [[ ! -n "$INPUT" ]]; then
	echo "$BASENAME: \`--input' argument is missing"
	echo $TRY_HELP
	exit 1
fi

if [[ ! -n "$OUT_DEB" ]] && [[ ! -n "$OUT_RPM" ]]; then
	echo "$BASENAME: output file argument is missing"
	echo $TRY_HELP
	exit 1
fi

if [[ ! -n "$COMMITER" ]]; then
	echo "$BASENAME: commiter information is missing"
	echo $TRY_HELP
	exit 1
fi

if [[ -n "$OUT_DEB" ]] && [[ ! -n "$PACKAGE" ]]; then
	echo "$BASENAME: package name is missing"
	echo $TRY_HELP
	exit 1
fi

if [[ ! -f "$INPUT" ]]; then
	echo "$BASENAME: cannot open \`$INPUT' (No such file or directory)"
	echo $TRY_HELP
	exit 1
fi

if [[ -n "$OUT_RPM" ]]; then
	rm -rf $OUT_RPM
	while IFS= read -r line <&3; do
		if [[ "$line" =~ \*[[:space:]]([0-9\.]+),[[:space:]](.*) ]]; then
			echo "* `date --date=\"${BASH_REMATCH[2]}\" +\"%a %b %d %Y\"` $COMMITER ${BASH_REMATCH[1]}-1" >> $OUT_RPM
		elif [[ "$line" =~ \-[[:space:]](.*) ]]; then
			echo "- ${BASH_REMATCH[1]}" >> $OUT_RPM
		else
			echo "$line" >> $OUT_RPM
		fi
	done 3< $INPUT
fi

END_LINE=""
if [[ -n "$OUT_DEB" ]]; then
	rm -rf $OUT_DEB
	while IFS= read -r line <&3; do
		if [[ "$line" =~ \*[[:space:]]([0-9\.]+),[[:space:]](.*) ]]; then
			if [[ "$END_LINE" != "" ]]; then
				echo "$END_LINE" >> $OUT_DEB
				END_LINE=""
				echo "" >> $OUT_DEB
			fi
			echo "$PACKAGE (${BASH_REMATCH[1]}-1) unstable; urgency=low" >> $OUT_DEB
			echo "" >> $OUT_DEB
			END_LINE=" -- $COMMITER  `date --date=\"${BASH_REMATCH[2]}\" +\"%a, %d %b %Y\"` 00:00:00 +0000"
		elif [[ "$line" =~ \-[[:space:]](.*) ]]; then
			echo "  * ${BASH_REMATCH[1]}" >> $OUT_DEB
		elif [[ "$line" != "" ]]; then
			echo "  $line" >> $OUT_DEB
		else
			echo "$line" >> $OUT_DEB
		fi
	done 3< $INPUT
	if [[ "$END_LINE" != "" ]]; then
		echo "" >> $OUT_DEB
		echo "$END_LINE" >> $OUT_DEB
		END_LINE=""
		echo "" >> $OUT_DEB
	fi
fi

exit 0

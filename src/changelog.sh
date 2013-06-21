#! /bin/bash

# exit in case of errors
set -e

BASENAME=`basename $0`
TRY_HELP="Try \`$BASENAME --help' for more information"

echo_help()
{
	echo "Usage:  $BASENAME [options]"
	echo '   -i, --input FILE                input changelog file'
	echo '   -d, --deb FILE                  output changelog file in deb format'
	echo '   -r, --rpm FILE                  output changelog file in rpm format'
	echo '   -c, --commiter "NAME <EMAIL>"   comitter name and email'
	echo '   -p, --package NAME              package name'
	echo '   -f, --force-version VERSION     latest version if not found in changelog'
	echo '   -h, --help                      print this message'
	echo
	echo 'Example:'
	echo "    $BASENAME -i ChangeLog -c \"John Doe <john@doe.com>\" -p coolapp"
	echo "              -r rpm.changes -d debian.changelog -f 0.1"
	echo "Version 0.3.2"
	echo 'Copyright (C) 2011 Sergey Vlasov <sergey@vlasov.me>'
}

args=`getopt -n$BASENAME -o i:d:r:c:p:f:h --longoptions="input: deb: rpm: commiter: package: force-version: help" -- "$@"`
if test $? != 0; then
	echo $TRY_HELP
	exit 1
fi

eval set -- $args
while [[ $# -gt 0 ]]; do
	if [[ $1 == "-i" || $1 == "--input" ]]; then
		shift; INPUT=$1
	elif [[ $1 == "-d" || $1 == "--deb" ]]; then
		shift; OUT_DEB=$1
	elif [[ $1 == "-r" || $1 == "--rpm" ]]; then
		shift; OUT_RPM=$1
	elif [[ $1 == "-c" || $1 == "--commiter" ]]; then
		shift; COMMITER=$1
	elif [[ $1 == "-p" || $1 == "--package" ]]; then
		shift; PACKAGE=$1
	elif [[ $1 == "-f" || $1 == "--force-version" ]]; then
		shift; USE_VERSION=$1
	elif [[ $1 == "-h" || $1 == "--help" ]]; then
		echo_help; exit 0
	elif [[ $1 == "--" ]]; then
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

FOUND_VERSION=no
FIRST_LINE=""

if [[ -n "$OUT_DEB" ]]; then
	rm -rf $OUT_DEB
	while IFS= read -r line <&3; do
		if [[ ! -n "$FIRST_LINE" ]]; then
			FIRST_LINE=yes
		elif [[ "$FIRST_LINE" == "yes" ]]; then
			FIRST_LINE=no
		fi
		if [[ "$line" =~ \*[[:space:]]([0-9\.]+),[[:space:]](.*) ]]; then
			if [[ "$FIRST_LINE" == "yes" ]]; then
				FOUND_VERSION=yes
			fi
			if [[ "$FIRST_LINE" != "yes" && "$FOUND_VERSION" != yes && ! -n "$USE_VERSION" ]]; then
				echo "$BASENAME: cannot detect version from \`$INPUT', use '--use-version' option"
				echo $TRY_HELP
				exit 1
			fi
			if [[ "$END_LINE" != "" ]]; then
				echo "$END_LINE" >> $OUT_DEB
				END_LINE=""
				echo "" >> $OUT_DEB
			fi
			echo "$PACKAGE (${BASH_REMATCH[1]}-1) unstable; urgency=low" >> $OUT_DEB
			echo "" >> $OUT_DEB
			END_LINE=" -- $COMMITER  `date --date=\"${BASH_REMATCH[2]}\" +\"%a, %d %b %Y\"` 00:00:00 +0000"
			continue
		elif [[ "$END_LINE" == "" ]]; then
			echo "" >> $OUT_DEB
			END_LINE=" -- $COMMITER  `date +\"%a, %d %b %Y %H:%m:%S\"` +0000"
		fi

		if [[ "$line" =~ \-[[:space:]](.*) ]]; then
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
	if [[ "$FOUND_VERSION" == "no" && -n "$USE_VERSION" ]]; then
		sed -i "1i $PACKAGE ($USE_VERSION-1) unstable; urgency=low" $OUT_DEB
	fi
fi

exit 0


#!/bin/bash

#
# Minimalistic test suite for the regular expression 'err-pattern'
#

#set -x

# Default pattern from src/keyfiles.c
PATTERN=$(grep err-pattern ../src/keyfiles.c | cut -d\" -f4)

# override with a TEST patten
PATTERN="((?<!no )error|(?<!insserv: )warning(?!: Linux headers are missing, which may explain the above failures)|failed|(?<!Linux headers are missing, which may explain the above )fail(?!2ban))"

# display all test data, with color highlightinhg.

echo "Lines that should remain white"
grep --color=always --text --ignore-case --no-messages --perl-regexp --regexp="$PATTERN" logtest-regular.txt logtest-falsepositives.txt
echo
echo "Lines that should get a highlight"
grep --color=always --text --ignore-case --no-messages --perl-regexp --regexp="$PATTERN|$" logtest-notices.txt


echo
echo "TEST:"
echo -n "regular log lines that should NOT match, expect 0 ... got "
grep --count --text --ignore-case --no-messages --perl-regexp --regexp="$PATTERN" logtest-regular.txt

echo -n "falsepositive lines that should NOT match, expect 0 ... got "
grep --count --text --ignore-case --no-messages --perl-regexp --regexp="$PATTERN" logtest-falsepositives.txt

echo -n "lines that should ALL match, expect `wc -l logtest-notices.txt` ... got "
grep --count --text --ignore-case --no-messages --perl-regexp --regexp="$PATTERN" logtest-notices.txt


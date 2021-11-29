#!/bin/sh

function test
{
    echo "$1"
    expected=$(cat $2 | dash)
    tmp2=$(cat $2 | ../builddir/42sh)
    if [ "$expected" = "$tmp2" ]; then
        echo "$(tput setaf 2)OK"
        tput sgr0
    else
        echo "$(tput setaf 1)FAILED"
        echo "expected : "
        echo "$(tput setaf 3)$expected"
        echo "$(tput setaf 1)but got :"
        echo "$(tput setaf 3)$tmp2"
        tput sgr0

        tput sgr0
    fi
    echo ""
}

echo "" && test "--IF THEN CMD--" "test_files/test1"
echo "" && test "--IF THEN ELSE--" "test_files/test2"
echo "" && test "--IF THEN ELIF THEN ELSE--" "test_files/test3"


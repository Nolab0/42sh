#!/bin/sh

function test
{
    echo "$1"
    tmp1=$(find $2)
    tmp2=$(cat exec_files/$2)
    if [ "$tmp1" = "$tmp2" ]; then
        echo "$(tput setaf 2)OK"
        tput sgr0
    else
        echo "$(tput setaf 1)FAILED"
        tput sgr0
    fi
    echo ""
}

echo "" && test "--BASIC FIND NO ARGS--" ""

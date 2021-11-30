#!/bin/sh

function test
{
    echo "$1"
    expected=$(cat $2 | dash)
    retexp=$?
    tmp2=$(cat $2 | ../builddir/42sh)
    ret=$?
    if [ "$expected" = "$tmp2" ] && [ $retexp -eq $ret ]; then
        echo "$(tput setaf 2)OK"
        tput sgr0
    else
        echo "$(tput setaf 1)FAILED"
        if [ $retexp -eq $ret ]; then
            echo "expected : "
            echo "$(tput setaf 3)$expected"
            echo "$(tput setaf 1)but got :"
            echo "$(tput setaf 3)$tmp2"
            tput sgr0
        else
            echo "expected : "
            echo "$(tput setaf 3)$retexp"
            echo "$(tput setaf 1)but got :"
            echo "$(tput setaf 3)$ret"
            tput sgr0
        fi

        tput sgr0
    fi
    echo ""
}

echo "" && test "--IF THEN CMD--" "test_files/test1"
echo "" && test "--IF THEN ELSE--" "test_files/test2"
echo "" && test "--IF THEN ELIF THEN ELSE--" "test_files/test3"
echo "" && test "--REDIR DEV NULL--" "test_files/test4"
echo "" && test "--REDIR DEV NULL APPENDED--" "test_files/test5"
echo "" && test "--REDIR 2 INTO DEV NULL--" "test_files/test6"
echo "" && test "--REDIR IF ECHO IN &2--" "test_files/test7"
echo "" && test "--SIMPLE ECHO TR PIPE--" "test_files/test8"

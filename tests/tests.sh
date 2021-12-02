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
}

echo "" && test "1--IF THEN CMD--" "test_files/test1"
echo "" && test "2--IF THEN ELSE--" "test_files/test2"
echo "" && test "3--IF THEN ELIF THEN ELSE--" "test_files/test3"
echo "" && test "4--REDIR DEV NULL--" "test_files/test4"
echo "" && test "5--REDIR DEV NULL APPENDED--" "test_files/test5"
echo "" && test "6--REDIR 2 INTO DEV NULL--" "test_files/test6"
echo "" && test "7--REDIR IF ECHO IN &2--" "test_files/test7"
echo "" && test "8--SIMPLE ECHO TR PIPE--" "test_files/test8"
echo "" && test "9--SIMPLE &&--" "test_files/test9"
echo "" && test "10--SIMPLE ||--" "test_files/test10"
echo "" && test "11--IF TRUE || FALSE--" "test_files/test11"
echo "" && test "12--IF FALSE || TRUE--" "test_files/test12"
echo "" && test "13--IF TRUE && FALSE--" "test_files/test13"
echo "" && test "14--IF FALSE && TRUE--" "test_files/test14"
echo "" && test "15--IF TRUE && TRUE && FALSE--" "test_files/test15"
echo "" && test "16--IF FALSE || FALSE || FALSE--" "test_files/test16"
echo "" && test "17--IF FALSE || FALSE || TRUE--" "test_files/test17"
echo "" && test "18--IF FALSE ||| TRUE --" "test_files/test18"
echo "" && test "19--IF LS && ECHO TRUE --" "test_files/test19"
echo "" && test "20--IF FALSE || ECHO YES THEN LS || FALSE && ECHO OUI --" "test_files/test20"

#!/bin/sh

RED=; GREEN=; YELLOW=; BLUE=; BOLD=; RESET=;
case ${TERM} in
  '') ;;
  *)
      RED=`tput setaf 1`
      GREEN=`tput setaf 2`
      YELLOW=`tput setaf 3`
      BLUE=`tput setaf 6`
      BOLD=`tput bold`
      RESET=`tput sgr0`;;
esac

function test
{
    expected=$(cat "test_files/$1_res")
    tmp2=$(cat "test_files/$1" | tail -n +2 | ../builddir/42sh | tr -d '\0')
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
    fi
    echo ""
}

for file in $(ls test_files | grep -v ".*_res$")
do
    cat "test_files/$file" | head -n 1
    test "$file"
done

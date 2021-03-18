#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./chibicc "$input" > tmp.s || exit
  actual=$(mockasm --file_path tmp.s)

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi

  echo ""
}

assert 0 0
assert 42 42
assert 21 '5+20-4'

echo OK

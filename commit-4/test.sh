#!/bin/bash
total=0

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

  ((total=total+1))

  echo ""
}

assert 0 0
assert 42 42
assert 21 '5+20-4'
assert 41 ' 12 + 34 - 5 '

echo "All $total tests passed!"
echo OK

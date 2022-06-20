import subprocess


def assert_ret(expected, input):
    _ = subprocess.getoutput(f"./chibicc '{input}' > tmp.yas")
    _ = subprocess.getoutput("yamasm tmp.yas -o tmp")
    output = subprocess.getoutput("yamini tmp").strip()

    if output == expected:
        print(f"{input} => {expected}")
    else:
        print(f"\033[91mError: \033[mExpected {expected}, but got {output}")


def main():
    assert_ret("0", "{ return 0; }")
    assert_ret("42", "{ return 42; }")
    assert_ret("21", "{ return 5+20-4; }")
    assert_ret("41", "{ return 12 + 34 - 5 ; }")
    assert_ret("47", "{ return 5+6*7; }")
    assert_ret("15", "{ return 5*(9-6); }")
    assert_ret("4", "{ return (3+5)/2; }")
    assert_ret("10", "{ return -10+20; }")
    assert_ret("10", "{ return - -10; }")
    assert_ret("10", "{ return - - +10; }")

    assert_ret("0", "{ return 0==1; }")
    assert_ret("1", "{ return 42==42; }")
    assert_ret("1", "{ return 0!=1; }")
    assert_ret("0", "{ return 42!=42; }")

    assert_ret("1", "{ return 0<1; }")
    assert_ret("0", "{ return 1<1; }")
    assert_ret("0", "{ return 2<1; }")
    assert_ret("1", "{ return 0<=1; }")
    assert_ret("1", "{ return 1<=1; }")
    assert_ret("0", "{ return 2<=1; }")

    assert_ret("1", "{ return 1>0; }")
    assert_ret("0", "{ return 1>1; }")
    assert_ret("0", "{ return 1>2; }")
    assert_ret("1", "{ return 1>=0; }")
    assert_ret("1", "{ return 1>=1; }")
    assert_ret("0", "{ return 1>=2; }")

    assert_ret("3", "{ a=3;  return a; }")
    assert_ret("8", "{ a=3; z=5;  return a+z; }")
    assert_ret("3", "{ foo=3;  return foo; }")
    assert_ret("8", "{ foo123=3; bar=5;  return foo123+bar; }")

    assert_ret("1", "{ return 1; 2; 3; }")
    assert_ret("2", "{ 1; return 2; 3; }")
    assert_ret("3", "{ 1; 2; return 3; }")
    assert_ret("5", "{ ;;; return 5; }")

    assert_ret("3", "{ if (0) return 2; return 3; }")
    assert_ret("3", "{ if (1-1) return 2; return 3; }")
    assert_ret("2", "{ if (1) return 2; return 3; }")
    assert_ret("2", "{ if (2-1) return 2; return 3; }")
    assert_ret("4", "{ if (0) { 1; 2; return 3; } else { return 4; } }")
    assert_ret("3", "{ if (1) { 1; 2; return 3; } else { return 4; } }")

    assert_ret("55", "{ i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }")
    assert_ret("3", "{ for (;;) {return 3;} return 5; }")

    
if __name__ == "__main__":
    main()
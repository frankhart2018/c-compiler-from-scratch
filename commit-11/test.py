import subprocess


def assert_ret(expected, input):
    _ = subprocess.getoutput(f"./chibicc '{input}' > tmp.yas")
    _ = subprocess.getoutput("yamasm tmp.yas tmp")
    output = subprocess.getoutput("yamini tmp").strip()

    if output == expected:
        print(f"{input} => {expected}")
    else:
        print(f"Expected {expected}, but got {output}")


def main():
    assert_ret("0", "0;")
    assert_ret("42", "42;")
    assert_ret("21", "5+20-4;")
    assert_ret("41", "12 + 34 - 5 ;")
    assert_ret("47", "5+6*7;")
    assert_ret("15", "5*(9-6);")
    assert_ret("4", "(3+5)/2;")
    assert_ret("10", "-10+20;")
    assert_ret("10", "- -10;")
    assert_ret("10", "- - +10;")

    assert_ret("0", "0==1;")
    assert_ret("1", "42==42;")
    assert_ret("1", "0!=1;")
    assert_ret("0", "42!=42;")

    assert_ret("1", "0<1;")
    assert_ret("0", "1<1;")
    assert_ret("0", "2<1;")
    assert_ret("1", "0<=1;")
    assert_ret("1", "1<=1;")
    assert_ret("0", "2<=1;")

    assert_ret("1", "1>0;")
    assert_ret("0", "1>1;")
    assert_ret("0", "1>2;")
    assert_ret("1", "1>=0;")
    assert_ret("1", "1>=1;")
    assert_ret("0", "1>=2;")

    assert_ret("3", "1;2;3;")

    assert_ret("3", "a=3; a;")
    assert_ret("8", "a=3; z=5; a+z;")
    assert_ret("3", "foo=3; foo;")
    assert_ret("8", "foo123=3; bar=5; foo123+bar;")

    
if __name__ == "__main__":
    main()
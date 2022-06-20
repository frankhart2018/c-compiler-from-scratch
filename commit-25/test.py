import subprocess


def inject_code(filename, code, after):
    with open(filename, "r") as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if after in line:
            lines.insert(i+1, code)
            break

    with open(filename, "w") as f:
        f.writelines(lines)


def assert_ret(expected, input):
    _ = subprocess.getoutput(f"./chibicc '{input}' > tmp.yas")

    inject_code(filename="tmp.yas", code="%ret3\nLOAD 3\nRET\n", after="JMP %start")
    inject_code(filename="tmp.yas", code="%ret5\nLOAD 5\nRET\n", after="JMP %start")
    inject_code(filename="tmp.yas", code="%add\nADD\nRET\n", after="JMP %start")
    inject_code(filename="tmp.yas", code="%sub\nSUB\nRET\n", after="JMP %start")
    inject_code(filename="tmp.yas", code=f"%add6\nADD\nADD\nADD\nADD\nADD\nRET\n", after="JMP %start")

    _ = subprocess.getoutput("yamasm tmp.yas -o tmp")
    output = subprocess.getoutput("yamini tmp").strip()

    if output == expected:
        print(f"{input} => {expected}")
    else:
        print(f"\033[91mError: \033[mExpected {expected}, but got {output}")


def main():
    assert_ret("0", "int main() { return 0; }")
    assert_ret("42", "int main() { return 42; }")
    assert_ret("21", "int main() { return 5+20-4; }")
    assert_ret("41", "int main() { return 12 + 34 - 5 ; }")
    assert_ret("47", "int main() { return 5+6*7; }")
    assert_ret("15", "int main() { return 5*(9-6); }")
    assert_ret("4", "int main() { return (3+5)/2; }")
    assert_ret("10", "int main() { return -10+20; }")
    assert_ret("10", "int main() { return - -10; }")
    assert_ret("10", "int main() { return - - +10; }")

    assert_ret("0", "int main() { return 0==1; }")
    assert_ret("1", "int main() { return 42==42; }")
    assert_ret("1", "int main() { return 0!=1; }")
    assert_ret("0", "int main() { return 42!=42; }")

    assert_ret("1", "int main() { return 0<1; }")
    assert_ret("0", "int main() { return 1<1; }")
    assert_ret("0", "int main() { return 2<1; }")
    assert_ret("1", "int main() { return 0<=1; }")
    assert_ret("1", "int main() { return 1<=1; }")
    assert_ret("0", "int main() { return 2<=1; }")

    assert_ret("1", "int main() { return 1>0; }")
    assert_ret("0", "int main() { return 1>1; }")
    assert_ret("0", "int main() { return 1>2; }")
    assert_ret("1", "int main() { return 1>=0; }")
    assert_ret("1", "int main() { return 1>=1; }")
    assert_ret("0", "int main() { return 1>=2; }")

    assert_ret("3", "int main() { int a; a=3;  return a; }")
    assert_ret("3", "int main() { int a=3; return a; }")
    assert_ret("8", "int main() { int a=3; int z=5;  return a+z; }")


    assert_ret("3", "int main() { int foo=3;  return foo; }")
    assert_ret("8", "int main() { int foo123=3; int bar=5;  return foo123+bar; }")

    assert_ret("1", "int main() { return 1; 2; 3; }")
    assert_ret("2", "int main() { 1; return 2; 3; }")
    assert_ret("3", "int main() { 1; 2; return 3; }")
    assert_ret("5", "int main() { ;;; return 5; }")

    assert_ret("3", "int main() { if (0) return 2; return 3; }")
    assert_ret("3", "int main() { if (1-1) return 2; return 3; }")
    assert_ret("2", "int main() { if (1) return 2; return 3; }")
    assert_ret("2", "int main() { if (2-1) return 2; return 3; }")
    assert_ret("4", "int main() { if (0) { 1; 2; return 3; } else { return 4; } }")
    assert_ret("3", "int main() { if (1) { 1; 2; return 3; } else { return 4; } }")

    assert_ret("55", "int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }")
    assert_ret("3", "int main() { for (;;) {return 3;} return 5; }")

    assert_ret("10", "int main() { int i=0; while(i<10) { i=i+1; } return i; }")

    assert_ret("3", "int main() { int x=3; return *&x; }")
    assert_ret("3", "int main() { int x=3; int *y=&x; int **z=&y; return **z; }")
    assert_ret("5", "int main() { int x=3; int y=5; return *(&x+1); }")
    assert_ret("3", "int main() { int x=3; int y=5; return *(&y-1); }")
    assert_ret("5", "int main() { int x=3; int y=5; return *(&x-(-1)); }")
    assert_ret("5", "int main() { int x=3; int *y=&x; *y=5; return x; }")
    assert_ret("7", "int main() { int x=3; int y=5; *(&x+1)=7; return y; }")
    assert_ret("7", "int main() { int x=3; int y=5; *(&y-2+1)=7; return x; }")
    assert_ret("5", "int main() { int x=3; return (&x+2)-&x+3; }")

    assert_ret("3", "int main() { return ret3(); }")
    assert_ret("5", "int main() { return ret5(); }")
    assert_ret("8", "int main() { return add(3, 5); }")
    assert_ret("2", "int main() { return sub(5, 3); }")
    assert_ret("21", "int main() { return add6(1, 2, 3, 4, 5, 6); }")
    assert_ret("66", "int main() { return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }")
    assert_ret("136", "int main() { return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }")
    
    
if __name__ == "__main__":
    main()
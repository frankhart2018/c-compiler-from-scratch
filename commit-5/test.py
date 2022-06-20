import subprocess


def assert_ret(expected, input):
    _ = subprocess.getoutput(f"./chibicc '{input}' > tmp.yas")
    _ = subprocess.getoutput("yamasm tmp.yas -o tmp")
    output = subprocess.getoutput("yamini tmp").strip()

    if output == expected:
        print(f"{input} => {expected}")
    else:
        print(f"Expected {expected}, but got {output}")


def main():
    assert_ret("0", "0")
    assert_ret("42", "42")
    assert_ret("21", "5+20-4")
    assert_ret("41", "12 + 34 - 5")
    assert_ret("47", "5+6*7")
    assert_ret("15", "5*(9-6)")
    assert_ret("4", "(3+5)/2")

    
if __name__ == "__main__":
    main()
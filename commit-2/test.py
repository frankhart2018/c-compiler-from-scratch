import subprocess


def assert_ret(expected, input):
    _ = subprocess.getoutput(f"./chibicc {input} > tmp.yas")
    _ = subprocess.getoutput("yamasm tmp.yas tmp")
    output = subprocess.getoutput("yamini tmp").strip()

    if output == expected:
        print(f"{input} => {expected}")
    else:
        print(f"Expected {expected}, but got {output}")


def main():
    assert_ret("0", "0")
    assert_ret("42", "42")
    assert_ret("21", "5+20-4")

    
if __name__ == "__main__":
    main()
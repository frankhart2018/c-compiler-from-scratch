import subprocess


def assert_ret(input, expected):
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

    
if __name__ == "__main__":
    main()
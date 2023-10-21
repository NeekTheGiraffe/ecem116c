import subprocess, textwrap

TESTS = [
    # Filename, expected output
    ('23instMem-r.txt', b'(-8,23)\n'),
    ('23instMem-sw.txt', b'(9,17)\n'),
    ('23instMem-all.txt', b'(40,1)\n'),
    ('sign-extension.txt', b'(-7,-26)\n'),
    ('sign-extension2.txt', b'(-61,0)\n'),
    ('write-x0.txt', b'(0,0)\n'),
]
EXECUTABLE_PATH = './cpusim'
TESTS_DIRECTORY = 'trace/'

def main():
    n_passed = 0
    for filename, expected in TESTS:
        result = subprocess.run([EXECUTABLE_PATH, f'{TESTS_DIRECTORY}{filename}'], stdout=subprocess.PIPE)
        if result.stdout == expected:
            print(f'========== {filename} ✅')
            n_passed += 1
        else:
            print(textwrap.dedent(f'''
                ========== {filename} ❌
                Expected: {expected.decode()}
                Received: {result.stdout.decode()}'''),
                end='')
    print(f'===== {n_passed}/{len(TESTS)} tests passed')
    if n_passed == len(TESTS):
        exit(0)
    else:
        exit(1)

if __name__ == '__main__':
    main()

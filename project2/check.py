import subprocess, textwrap
from functools import reduce

TESTS = [
    # Filename, expected output
    ('mini_debug.txt', 0.55, 1.0, 60.95),
    ('victim-debug.txt', 0.79167, 1.0, 78.29167),
    ('L2-test.txt', 0.88889, 0.83333, 83.07407),
]
EXECUTABLE_PATH = './memory_driver'
TESTS_DIRECTORY = 'traces/'
EPSILON = 1e-4

def main():
    n_passed = 0
    for test in TESTS:
        filename, expected = test[0], test[1:]
        result = subprocess.run([EXECUTABLE_PATH, f'{TESTS_DIRECTORY}{filename}'], stdout=subprocess.PIPE)
        actual = extract_output(result.stdout.decode())
        if compare_output(expected, actual):
            print(f'========== {filename} ✅', end='')
            n_passed += 1
        else:
            print(textwrap.dedent(f'''
                ========== {filename} ❌
                Expected: {expected}
                Received: {actual}'''))
    print(f'===== {n_passed}/{len(TESTS)} tests passed')
    if n_passed == len(TESTS):
        exit(0)
    else:
        exit(1)

def extract_output(output: str):
    return tuple(float(x) for x in output.strip('()\n').split(','))

def compare_output(expected, actual):
    return reduce(lambda a, el: a and abs(el[0] - el[1]) < EPSILON,
                  zip(expected, actual),
                  True)

if __name__ == '__main__':
    main()

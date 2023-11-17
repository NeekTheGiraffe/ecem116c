import subprocess, textwrap
from functools import reduce
from argparse import ArgumentParser

TESTS = [
    # Filename, expected output
    ('mini_debug.txt', 0.25, 1.0, 28.25),
    ('victim-debug.txt', 0.72727, 1.0, 60.63636),
    ('L2-test.txt', 0.82353, 0.71429, 67.23529),
]
EXECUTABLE_PATH = './memory_driver'
TESTS_DIRECTORY = 'traces/'
EPSILON = 1e-4

def main():
    parser = ArgumentParser()
    parser.add_argument('-v', '--verbose', action='store_true')
    verbose = parser.parse_args().verbose

    n_passed = 0
    for test in TESTS:
        filename, expected = test[0], test[1:]
        result = subprocess.run([EXECUTABLE_PATH, f'{TESTS_DIRECTORY}{filename}'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        actual = extract_output(result.stdout.decode())
        stderr = result.stderr.decode()
        if compare_output(expected, actual):
            print(f'========== {filename} ✅')
            if verbose and len(stderr) > 0:
                print(f'stderr:\n{stderr}')
            n_passed += 1
        else:
            print(textwrap.dedent(f'''\
                ========== {filename} ❌
                Expected: {expected}
                Received: {actual}'''))
            if len(stderr) > 0:
                print(f'stderr:\n{stderr}')
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

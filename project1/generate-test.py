from riscv_assembler.convert import AssemblyConverter as AC
from argparse import ArgumentParser

# TODO: The AssemblyConverter may not output the correct machine code for sw
#   instructions. See trace/load-and-store.* 
def main():
    parser = ArgumentParser()
    parser.add_argument('infile')
    parser.add_argument('outfile')
    args = parser.parse_args()

    cnv = AC()
    cnv.convert(args.infile)

    with open(args.outfile, 'w') as outfile:
        for instruction in cnv.convert(args.infile):
            for i in [24, 16, 8, 0]:
                outfile.write(f'{str(int(instruction[i:i+8], 2))}\n')

if __name__ == '__main__':
    main()
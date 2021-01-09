#!/usr/bin/env python3

# generate.py generates Markdown files from the comments of C source files. If the first line in a C file is
# //genexample, it will stitch the code and comments into a Markdown file of the same name. Additionally, if a PNG
# file with the same name is present in the folder, generate.py will include it in the document.

from pathlib import Path


def main():
    example_dir = Path(__file__).parent
    for example_src in example_dir.glob('*.c'):
        lines = example_src.open('r').readlines()

        # Some files in examples/ may be common utility code, so don't generate corresponding Markdown files
        if not lines[0].strip().startswith('//genexample'):
            continue

        with example_src.with_suffix('.md').open('w') as out:
            out.write(f'# {example_src.stem}\n')

            if (img := example_src.with_suffix('.png')).is_file():
                out.write(f'![Example Screenshot]({img.relative_to(example_dir)})\n\n')

            in_code_block = False
            for line in lines:
                if (s := line.lstrip()).startswith('//'):
                    if in_code_block:
                        out.write('```\n\n')
                        in_code_block = False
                    out.write(s[2:].lstrip())
                elif not in_code_block:
                    out.write('```c\n')
                    in_code_block = True

                if in_code_block and line.strip():
                    out.write(line)

            if in_code_block:
                out.write('```\n')

            out.write('\n')
            out.write(f'###### Automatically generated from [{example_src.name}]({example_src.name})\n')

        print(example_src)


if __name__ == '__main__':
    main()

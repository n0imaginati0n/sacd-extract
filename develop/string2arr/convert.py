#!/usr/bin/env python3

# convert string initialization of the 4-char id3tag arrays
# with appropriate array initialization

if __name__ == '__main__':
	in_file = "from.txt"
	out_file = "to.txt"

	with open(in_file, "r") as inp, open(out_file, "w") as otp:
		for l in inp.readlines():
			toks = l.split(',')
			if len(toks) > 2:
				src = toks[1].strip().strip('"')
				toks[1] = " {'" + "', '".join(src) + "'}"

			otp.write(', '.join(toks))

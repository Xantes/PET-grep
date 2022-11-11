# PET-grep
The repository is PET-project, that implements standard "grep" function in C language

Usage:

s21_grep [-benst] [file ...] 

  -i, --ignore-case               Ignore case distinctions in pattern and input data. \
  -o, --only-matching             Print only the matched (non-empty) parts of a matching line \
  -v, --invert-match              Invert matching, to select non-matching lines\
  -c, --count                     Suppress normal output; instead print a count of matching lines for each input file\
  -l, --files-with-matches        Print the name of each input file instead of normal output \
  -n, --line-number               Prefix each line of output with line number \
  -h, --no-filename               Suppress the prefixing of file names on output \
  -s, --no-messages               Silent mode \
  -f FILE, --file=FILE            Obtain patterns from FILE, one per line. Can be used multiple times
  -e PATTERN, --regexp=PATTERNS   Use PATTERNS as the patterns. Can be used multiple times

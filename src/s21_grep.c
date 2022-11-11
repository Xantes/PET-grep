#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MSG_LEN 1024

typedef struct format_s {
  bool ext_pattern;
  bool ignore_case;
  bool inverted;
  bool match_counter;
  bool only_filenames;
  bool line_counter;
  bool no_filenames;
  bool no_error_msg;
  bool only_pattern;
  bool m_files;
} format;

void grep_function(regex_t *, char *, int, format);
void compile_reg(regex_t **, char *, int *, format);
void print_usage();
void print_no_file(char *, format);

int main(int argc, char **argv) {
  /* VARIABLES DECLARATION */

  const char *short_options = "iovclnhsf:e:";

  static const struct option long_options[] = {
      {"count", no_argument, NULL, 'c'},
      {"regexp", required_argument, NULL, 'e'},
      {"file", required_argument, NULL, 'f'},
      {"no-filename", no_argument, NULL, 'h'},
      {"ignore-case", no_argument, NULL, 'i'},
      {"files-with-matches", no_argument, NULL, 'l'},
      {"line-number", no_argument, NULL, 'n'},
      {"only-matching", no_argument, NULL, 'o'},
      {"no-messages", no_argument, NULL, 's'},
      {"invert-match", no_argument, NULL, 'v'},
      {NULL, no_argument, NULL, 0}};

  errno = 0;
  int rez = 0;
  char *filename = NULL;
  format grep_opt = {0};
  errno = 0;
  regex_t *patterns = NULL;
  int num_patterns = 0;
  int line_size = 0;
  FILE *f_descriptor;
  char *line_buf;
  size_t line_buf_size = 256;
  /* END VARIABLES DECLARATION*/

  /* if no argument is passed */
  if (argc == 1) {
    /*set errno = 1 */
    print_usage();
  }

  while (!errno && ((rez = getopt_long(argc, argv, short_options, long_options,
                                       NULL)) != -1)) {
    switch (rez) {
      case 'e':
        compile_reg(&patterns, optarg, &num_patterns, grep_opt);
        break;
      case 'i':
        grep_opt.ignore_case = true;
        break;
      case 'v':
        grep_opt.inverted = true;
        break;
      case 'c':
        grep_opt.match_counter = true;
        break;
      case 'l':
        grep_opt.only_filenames = true;
        break;
      case 'n':
        grep_opt.line_counter = true;
        break;
      case 'h':
        grep_opt.no_filenames = true;
        grep_opt.m_files = false;
        break;
      case 's':
        grep_opt.no_error_msg = true;
        break;
      case 'f':
        grep_opt.ext_pattern = true;
        filename = optarg;
        if (access(filename, F_OK) == 0) {
          if ((f_descriptor = fopen(filename, "r")) != NULL) {
            line_buf = calloc(line_buf_size, sizeof(char));
            while ((line_size = getline(&line_buf, &line_buf_size,
                                        f_descriptor)) != -1) {
              if (line_buf[line_size - 1] == '\n' && line_size > 1) {
                line_buf[line_size - 1] = 0;
              }
              compile_reg(&patterns, line_buf, &num_patterns,
                          grep_opt);  // check multiple files with pattern
            }
            if (line_buf) {
              free(line_buf);
            }
          }
          if (f_descriptor) {
            fclose((f_descriptor));
          }
        } else {
          print_no_file(filename, grep_opt);  // check prtint_no_file func
        }

        break;
      case 'o':
        grep_opt.only_pattern = true;
        break;
      default:
        /* set errno = 1 */
        print_usage();
    }
  }

  if (!errno && (!grep_opt.ext_pattern || !grep_opt.m_files)) {
    if (!num_patterns) {
      compile_reg(&patterns, argv[optind], &num_patterns, grep_opt);
      optind++;
    }
  }

  if (!errno && (optind != argc - 1 && !grep_opt.no_filenames)) {
    grep_opt.m_files = true;
  }

  if (!errno) {
    while (optind < argc) {
      filename = argv[optind];
      if (access(filename, F_OK) == 0) {
        grep_function(patterns, filename, num_patterns, grep_opt);
      } else {
        print_no_file(filename, grep_opt);
      }
      optind++;
    }
  }

  if (patterns) {
    while (num_patterns) {
      num_patterns--;
      regfree(&patterns[num_patterns]);
    }
    // regfree(patterns);
    free(patterns);
  }

  return errno;
}

void grep_function(regex_t *patterns, char *filename, int num_patterns,
                   format grep_opt) {
  /* VARIABLES DECLARATION */
  FILE *f_descriptor;
  char *line_buf;
  size_t line_buf_size = 256;
  int reg_result = 0;
  int line_size = 0;
  int match_counter = 0;
  int line_counter = 0;

  size_t nregmatch = 5;
  regmatch_t regmatch[5];
  /* END VARIABLES DECLARATION*/

  if ((f_descriptor = fopen(filename, "r")) == NULL) {
    print_no_file(filename, grep_opt);
  }

  if (!errno && f_descriptor) {
    line_buf = calloc(line_buf_size + 1, sizeof(char));

    while (!errno && ((line_size = getline(&line_buf, &line_buf_size,
                                           f_descriptor)) != -1)) {
      line_counter++;
      bool has_match = false;
      for (int i = 0; i < num_patterns; i++) {
        reg_result = regexec(&patterns[i], line_buf, nregmatch, regmatch, 0);

        if ((!reg_result && !grep_opt.inverted) ||
            (reg_result == REG_NOMATCH && grep_opt.inverted)) {
          if (!has_match) {
            match_counter++;
          }
          if (!grep_opt.match_counter && !grep_opt.only_filenames &&
              !has_match) {
            if (grep_opt.m_files) {
              fprintf(stdout, "%s:", filename);
            }
#ifdef OS_DARWIN_
            if (grep_opt.line_counter) {
              fprintf(stdout, "%d:", line_counter);
            }
#endif

            if (grep_opt.only_pattern && !grep_opt.inverted) {
              int chunk = 0;
              char *sub_line_buf = line_buf + chunk;
              while (!(regexec(&patterns[i], sub_line_buf, nregmatch, regmatch,
                               0))) {
                chunk += (int)regmatch[0].rm_eo;
                fprintf(stdout, "%.*s\n",
                        (int)(regmatch[0].rm_eo - regmatch[0].rm_so),
                        &sub_line_buf[regmatch[0].rm_so]);
                sub_line_buf = line_buf + chunk;
              }
              continue;
            }
            fprintf(stdout, "%s", line_buf);

            if (line_buf[line_size - 1] == '\n') {
            } else {
              fprintf(stdout, "\n");
            }
          }
          has_match = true;
        }
      }
    }
    if (!errno) {
      if (grep_opt.match_counter && grep_opt.only_filenames) {
        if (grep_opt.m_files) {
          fprintf(stdout, "%s:", filename);
        }
        if (match_counter) {
#ifdef OS_DARWIN_
          fprintf(stdout, "1\n");
#endif
          fprintf(stdout, "%s\n", filename);
        } else {
          fprintf(stdout, "0\n");
        }

      } else {
        if (grep_opt.only_filenames) {
          if (!grep_opt.match_counter && match_counter > 0) {
            fprintf(stdout, "%s\n", filename);
          }
          if (grep_opt.match_counter) {
            fprintf(stdout, "%s:", filename);
          }
        }
        if (grep_opt.match_counter) {
          if (grep_opt.m_files && !grep_opt.only_filenames) {
            fprintf(stdout, "%s:", filename);
          }
          fprintf(stdout, "%d\n", match_counter);
        }
      }
    }
    if (line_buf) {
      free(line_buf);
    }
  }
  if (f_descriptor) {
    fclose(f_descriptor);
  }
}

void compile_reg(regex_t **patterns, char *pattern, int *num_patterns,
                 format grep_opt) {
  *patterns = realloc(*patterns, sizeof(regex_t) * (++(*num_patterns)));
  if (*patterns) {
    regcomp(&(*patterns)[*num_patterns - 1], pattern,
            grep_opt.ignore_case ? REG_ICASE | REG_EXTENDED | REG_NEWLINE
                                 : REG_NEWLINE | REG_EXTENDED);
  } else {
    free(*patterns);
  }
}

void print_usage() {
  fprintf(stderr, "usage: grep [options] template [file_name]");
  errno = 1;
}

void print_no_file(char *filename, format grep_opt) {
  char error_buf[MSG_LEN];
  int error_num = errno;
  strerror_r(error_num, error_buf, MSG_LEN);
  if (!grep_opt.no_error_msg) {
    fprintf(stderr, "%s: %s\n", filename, error_buf);
  }
  errno = 0;
}

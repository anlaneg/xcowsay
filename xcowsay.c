#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#include "display_cow.h"
#include "settings.h"
#include "xcowsayd.h"

// Default settings
#define DEF_LEAD_IN_TIME  250
#define DEF_DISPLAY_TIME  4000
#define DEF_LEAD_OUT_TIME LEAD_IN_TIME
#define DEF_FONT          "Bitstream Vera Sans 14" 

#define MAX_STDIN 4096   // Maximum chars to read from stdin

static int daemon_flag = 0;
static int daemon_debug = 0;

static struct option long_options[] = {
   {"help", no_argument, 0, 'h'},
   {"time", required_argument, 0, 't'},
   {"font", required_argument, 0, 'f'},
   {"daemon", no_argument, &daemon_flag, 1},
   {"debug", no_argument, &daemon_debug, 1},
   {0, 0, 0, 0}
};

static void read_from_stdin(void)
{
   char *data = malloc(MAX_STDIN);
   size_t n = fread(data, 1, MAX_STDIN, stdin);
   if (n == MAX_STDIN) {
      fprintf(stderr, "Warning: Excess input truncated\n");
      n--;
   }
   data[n] = '\0';

   display_cow(data);
   free(data);
}

static void usage()
{
   static const char *usage_message =
      "Usage: xcowsay [OPTION]... [MESSAGE]\n"
      "Display a cow on your desktop with MESSAGE or standard input.\n\n"
      "Options:\n"
      " -h, --help\t\tDisplay this message and exit.\n"
      " -t, --time=SECONDS\tDisplay message for SECONDS seconds.\n"
      " -f, --font=FONT\tSet message font (Pango format).\n"
      " -d, --daemon\t\tRun xcowsay in daemon mode.\n"
      "     --debug\t\tKeep daemon attached to terminal.\n";
   puts(usage_message);
}

static int parse_int_option(const char *optarg)
{
   char *endptr;
   int r = strtol(optarg, &endptr, 10);
   if ('\0' == *endptr)
      return r;
   else {
      fprintf(stderr, "Error: %s is not a valid integer\n", optarg);
      exit(EXIT_FAILURE);
   }
}

int main(int argc, char **argv)
{
   add_int_option("lead_in_time", DEF_LEAD_IN_TIME);
   add_int_option("display_time", DEF_DISPLAY_TIME);
   add_int_option("lead_out_time", get_int_option("lead_in_time"));
   add_string_option("font", DEF_FONT);
   
   int c, index = 0, failure = 0;
   const char *spec = "hdt:f:";
   while ((c = getopt_long(argc, argv, spec, long_options, &index)) != -1) {
      switch (c) {
      case 0:
         // Set a flag
         break;
      case 'd':
         daemon_flag = 1;
         break;
      case 'h':
         usage();
         exit(EXIT_SUCCESS);
      case 't':
         set_int_option("display_time", parse_int_option(optarg)*1000);
         break;
      case 'f':
         set_string_option("font", optarg);
         break;         
      case '?':
         // getopt_long already printed an error message
         failure = 1;
         break;
      default:
         abort();
      }
   }
   if (failure)
      exit(EXIT_FAILURE);

   srand((unsigned)time(NULL));

   if (daemon_flag) {
      run_cowsay_daemon(daemon_debug, argc, argv);
   }
   else {
      cowsay_init(&argc, &argv);
      
      if (optind == argc) {
         read_from_stdin();
      }
      else if (optind == argc - 1) {
         display_cow(argv[optind]);
      }
      else {
         fprintf(stderr, "Error: Too many arguments\n");
      }
   }
   
   return EXIT_SUCCESS;
}

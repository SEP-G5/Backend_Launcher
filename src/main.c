#include <argparse/argparse.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static const char *const usage[] = {"launcher [options] [[--] args]",
                                    "launcher [options]", NULL};

typedef struct {
  const char *path;
  int num;
} Data;

int run(const char *path, int num, int offset) {
  enum { kCmdLen = 512 };
  char cmd[kCmdLen];
  int node_port = 35010 + num + offset;
  int rest_port = 8000 + num + offset;
  snprintf(cmd, kCmdLen, "%s -n 0.0.0.0:%d -r %d", path, node_port, rest_port);

  int res = system(cmd);
  printf("\n~~~~~~~~~~~~~~~~~~~~~~\nProgram exited with code [%d]\n", res);
  return res;
}

int launcher(int node_count, const char *path, int offset) {
  printf("Launching %d nodes, with path\n[%s]\n", node_count, path);

  int *pids = malloc(sizeof(int) * node_count);
  int i;
  for (i = 0; i < node_count - 1; i++) {
    pids[i] = fork();
    if (pids[i] == 0) {
      fclose(stdout);
      fclose(stdin);
      return run(path, i, offset);
    } else {
      printf("Starting child %d.\n", i);
      const struct timespec req = {0, 1000 * 1000 * 250};
      struct timespec rem;
      nanosleep(&req, &rem);
    }
  }

  printf("Running myself as child %d.\n", i);
  int res = run(path, i, offset);

  printf("Killing children.\n");
  for (i = 0; i < node_count - 1; i++) {
    printf("\t%d died.\n", pids[i]);
    kill(pids[i], SIGKILL);
  }

  return res;
}

int main(int argc, const char **argv) {
  int node_count = 0;
  char *path = NULL;
  int offset = 0;

  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_INTEGER('n', "nodes", &node_count, "How many nodes to launch.", NULL,
                  0, 0),
      OPT_STRING('p', "path", &path, "Path to backend executable (full path).",
                 NULL, 0, 0),
      OPT_INTEGER('o', "offset", &offset, "Normally, the port starts from 35010 "
                  "and 8000, then increments by one for each node. Setting an "
                  "offset of 10 will have them start at 35020 and 8010.", NULL, 0, 0),
      OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, usage, 0);
  argparse_describe(&argparse, "\nLaunch n amount of nodes.", "\n");
  argc = argparse_parse(&argparse, argc, argv);

  if (node_count < 1) {
    printf("Node count not set, (or is invalid), defaulting to 2\n");
    node_count = 2;
  }
  if (!path) {
    printf("No path set, defaulting to ls\n");
    path = "ls";
  }
  if (offset < 0) {
    printf("Cannot have negative offset, defaulting to 0.\n");
    offset = 0;
  }

  return launcher(node_count, path, offset);
}

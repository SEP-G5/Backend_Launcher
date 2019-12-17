#include <argparse/argparse.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const char *const usage[] = {"launcher [options] [[--] args]",
                                    "launcher [options]", NULL};

typedef struct {
  const char *path;
  int num;
} Data;

int run(const char *path, int num) {
  enum { kCmdLen = 512 };
  char cmd[kCmdLen];
  int node_port = 35010 + num;
  int rest_port = 8000 + num;
  snprintf(cmd, kCmdLen, "%s -n 0.0.0.0:%d -r %d", path, node_port, rest_port);

  int res = system(cmd);
  printf("\n~~~~~~~~~~~~~~~~~~~~~~\nProgram exited with code [%d]\n", res);
  return res;
}

int launcher(int node_count, const char *path) {
  printf("Launching %d nodes, with path\n[%s]\n", node_count, path);

  int *pids = malloc(sizeof(int) * node_count);
  int i;
  for (i = 0; i < node_count - 1; i++) {
    pids[i] = fork();
    if (pids[i] == 0) {
      fclose(stdout);
      fclose(stdin);
      return run(path, i);
    } else {
      printf("Starting child %d.\n", i);
    }
  }

  printf("Running myself as child %d.\n", i);
  int res = run(path, i);

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

  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_INTEGER('n', "nodes", &node_count, "How many nodes to launch.", NULL,
                  0, 0),
      OPT_STRING('p', "path", &path, "Path to backend executable (full path).",
                 NULL, 0, 0),
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

  return launcher(node_count, path);
}

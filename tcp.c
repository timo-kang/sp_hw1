#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <alloca.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int main(int, char *[]);
static char *determine_target(const char *, const char *);
static void copy_file_buffer(int, int, size_t);
static void usage(void);

/*
 *  * Copies a source file to a target file.
 *   */
int
main(int argc, char *argv[])
{
  char *source_path;
  char *target_path;
  int source_fd;
  int target_fd;
  struct stat source_sb;
  struct stat target_sb;

  if (argc != 3)
    usage();
    /* NOTREACHED */

  /* Determine the paths. */
  source_path = argv[1];
  if ((stat(source_path, &source_sb) == -1) || !S_ISREG(source_sb.st_mode))
    err(EXIT_FAILURE, "source is not a regular file");
  target_path = determine_target(source_path, argv[2]);

  if (target_path == NULL) {
      if (errno == 0)
        errx(EXIT_FAILURE, "target path is invalid");
      else
        errx(EXIT_FAILURE, "target path is invalid");
    }

  if ((stat(target_path, &target_sb) != -1) && (source_sb.st_ino
			          == target_sb.st_ino))
    errx(EXIT_FAILURE, "source and target are the same file");

  /* Open the files. */
  if ((source_fd = open(source_path, O_RDONLY)) == -1)
    err(EXIT_FAILURE, "source open error");
  if ((target_fd = open(target_path, O_RDWR | O_CREAT | O_TRUNC,
				          source_sb.st_mode)) == -1)
    err(EXIT_FAILURE, "target open error");

  free(target_path);

  /* Copy source to target. */
  copy_file_buffer(source_fd, target_fd, source_sb.st_blksize);

  /* Close the files. */
  if (close(source_fd) == -1)
    err(EXIT_FAILURE, "source close error");
  if (close(target_fd) == -1)
    err(EXIT_FAILURE, "target close error");

  return EXIT_SUCCESS;
}

/*
 *  * Determines the target path name depending on the source path. If the target
 *   * is a directory, the returned string will be the given target path appended
 *    * by the source file name. If an error occurs, this function will return NULL.
 *     * Otherwise, this function returns the original target name.
 *      */
static char *
determine_target(const char *source, const char *target)
{
  struct stat sb;
  size_t target_length;
  char *target_path;

  assert(source != NULL);
  assert(target != NULL);

  target_length = strlen(target);

  if (target_length == 0)
    return NULL;

  target_path = (char *)malloc(sizeof(char) * (PATH_MAX + 1));

  if (target_path == NULL)
    err(EXIT_FAILURE, "Not enough memory for target name");

  if (stat(target, &sb) == -1) {
      /*
       *      * It is OK, if target refers to a non-existing file, because we may be
       *           * able to create it.
       *                */
      if (errno == ENOENT) {
            strcpy(target_path, target);
            return target_path;
          }
    } else if (S_ISDIR(sb.st_mode)) {
        char *source_dup;
        char *source_name;
        size_t source_name_length;
        int n;
        int n_expect;
    
        /*
	 *      * Target is a directory.
	 *           * Extract the file name from source and append to target.
	 *                */
        source_dup = strdup(source);
        source_name = basename(source_dup);
        source_name_length = strlen(source_name);
    
        /* Add trailing / to target, if missing. */
        if (strrchr(target, '/') == (target + target_length - 1)) {
	      n = snprintf(target_path, PATH_MAX + 1, "%s%s", target, source_name);
	      n_expect = target_length + source_name_length;
	    } else {
	          n = snprintf(target_path, PATH_MAX + 1, "%s/%s", target, source_name);
	          n_expect = target_length + 1 + source_name_length;
	        }
    
        free(source_dup);
    
        if (n == n_expect)
          return target_path;
      } else if (S_ISREG(sb.st_mode)) {
          strcpy(target_path, target);
          return target_path;
        }

  free(target_path);
  return NULL;
}

/*
 *  * Copies the source file to the target file using a buffer.
 *   */
static void
copy_file_buffer(int source_fd, int target_fd, size_t buffer_size)
{
  char *buffer = alloca(buffer_size);
  int n;

  while ((n = read(source_fd, buffer, buffer_size)) > 0) {
      if (write(target_fd, buffer, n) != n)
        err(EXIT_FAILURE, "write error");
    }
}

/*
 *  * Prints usage information and exits.
 *   */
static void
usage(void)
{
 //(void)fprintf(stderr, "usage: %s source target\n", getprogname());
  exit(EXIT_FAILURE);
}

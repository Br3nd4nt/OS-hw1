#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

const int MAX_BUFF_SIZE = 5000;

void print_usage(char *argv[]) {
  printf("Usage: %s -i input_file -o output_file\n", argv[0]);
}

int main(int argc, char *argv[]) {
  char *input_file = NULL;
  char *output_file = NULL;
  char line[MAX_BUFF_SIZE];

  int fd_first2second[2];
  int first_result;
  int fd_second2third[2];
  int second_result;
  int size;
  size_t message_size;

  if (argc != 5) {
    print_usage(argv);
    return 1;
  }

  for (int i = 1; i < argc; i += 2) {
    if (strcmp(argv[i], "-i") == 0) {
      input_file = argv[i + 1];
    } else if (strcmp(argv[i], "-o") == 0) {
      output_file = argv[i + 1];
    } else {
      print_usage(argv);
      return 1;
    }
  }

  if (input_file == NULL || output_file == NULL) {
    print_usage(argv);
    return 1;
  }

  if (pipe(fd_first2second) < 0) {
    printf("Cannot create pipe from first to second process");
    return 1;
  }

  if (pipe(fd_second2third) < 0) {
    printf("Cannot create pipe from second to third process");
    return 1;
  }

  first_result = fork();

  if (first_result < 0) {
    printf("Cannot fork to child");
    return 1;
  } else if (first_result > 0) { // first process
    // readnf file
    FILE *input_file_line = fopen(input_file, "r");

    if (input_file_line == NULL) {
      print_usage(argv);
    }

    if (fgets(line, MAX_BUFF_SIZE, input_file_line) == NULL) {
      print_usage(argv);
    }

    message_size = strlen(line);

    if (close(fd_first2second[0]) < 0) {
      printf("first: Can\'t close reading side of pipe\n");
      exit(-1);
    }
    size = write(fd_first2second[1], line, message_size);
  } else {
    if (close(fd_first2second[1]) < 0) {
      printf("second: Can\'t close writing side of pipe\n");
      exit(-1);
    }

    second_result = fork();
    if (second_result < 0) {
      printf("Cannot fork to child");
      return 1;
    } else if (second_result > 0) { // second
      char message_from_pipe[MAX_BUFF_SIZE];

      message_size = read(fd_first2second[0], message_from_pipe, MAX_BUFF_SIZE);

      // PROCCESS
      int counts[10] = {0};
      for (int i = 0; i < strlen(message_from_pipe); i++) {
        int digit = message_from_pipe[i] - '0';
        if (digit >= 0 && digit <= 9) {
          counts[digit]++;
        }
      }

      if (write(fd_second2third[1], &counts, sizeof(counts) + 1) == -1) {
        printf("error writing to pipe");
        exit(-1);
      }

    } else { // third proccess
      int recieved_data[10];
      if (read(fd_second2third[0], &recieved_data, sizeof(recieved_data)) ==
          -1) {
        printf("error reading data from pipe");
        exit(-1);
      }
      FILE *file_to_write = fopen(output_file, "w");
      if (file_to_write == NULL) {
        printf("Cannot open file to write");
        exit(-1);
      }

      for (int i = 0; i < 10; i++) {
        // printf("%d - %d\n", i, recieved_data[i]);
        fprintf(file_to_write, "%d - %d\n", i, recieved_data[i]);
      }
    }
  }

  return 0;
}

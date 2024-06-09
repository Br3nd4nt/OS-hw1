#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
  char first2second[] = "first2second.fifo";
  char second2third[] = "second2third.fifo";
  int first_result, second_result;
  int first_fd, second_fd;

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

  mknod(first2second, S_IFIFO | 0666, 0);
  mknod(second2third, S_IFIFO | 0666, 0);

  if ((first_result = fork()) < 0) {
    printf("Cannot fork to child");
    return 1;
  } else if (first_result > 0) { // first
    if ((first_fd = open(first2second, O_WRONLY)) < 0) {
      printf("Cannot open first2second fifo");
      return 1;
    }

    FILE *input_file_line = fopen(input_file, "r");

    if (input_file_line == NULL) {
      print_usage(argv);
    }

    if (fgets(line, MAX_BUFF_SIZE, input_file_line) == NULL) {
      print_usage(argv);
    }

    int message_size = strlen(line);

    int size = write(first_fd, line, message_size);
    if (close(first_fd) < 0) {
      printf("Cannot close first2second fifo");
      return 1;
    }

  } else {
    if ((second_result = fork()) < 0) {
      printf("Cannot fork to child");
      return 1;
    } else if (second_result > 0) { // second
      if ((first_fd = open(first2second, O_RDONLY)) < 0) {
        printf("Cannot open first2second fifo");
        return 1;
      }
      char read_message[MAX_BUFF_SIZE];
      int size = read(first_fd, read_message, MAX_BUFF_SIZE);

      if (close(first_fd) < 0) {
        printf("Cannot close first2second fifo");
        return 1;
      }

      int counts[10] = {0};
      for (int i = 0; i < strlen(read_message); i++) {
        int digit = read_message[i] - '0';
        if (digit >= 0 && digit <= 9) {
          counts[digit]++;
        }
      }

      if ((second_fd = open(second2third, O_WRONLY)) < 0) {
        printf("Cannot open second2third fifo");
        return 1;
      }

      if (write(second_fd, &counts, sizeof(counts) + 1) == -1) {
        printf("error writing to fifo");
        exit(-1);
      }

      if (close(second_fd) < 0) {
        printf("Cannot close second2third fifo");
        return 1;
      }

    } else { // third
      if ((second_fd = open(second2third, O_RDONLY)) < 0) {
        printf("Cannot open second2third fifo");
        return 1;
      }

      int recieved_data[10];
      if (read(second_fd, &recieved_data, sizeof(recieved_data)) == -1) {
        printf("error reading data from pipe");
        exit(-1);
      }
      if (close(second_fd) < 0) {
        printf("Cannot close second2third fifo");
        return 1;
      }
      FILE *file_to_write = fopen(output_file, "w");
      if (file_to_write == NULL) {
        printf("Cannot open file to write");
        exit(-1);
      }

      for (int i = 0; i < 10; i++) {
        fprintf(file_to_write, "%d - %d\n", i, recieved_data[i]);
      }
    }
  }
  return 0;
}

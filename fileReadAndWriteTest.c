#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include<unistd.h> 

int main()
{
  int fd, sz;
  char *c = (char *) calloc(100, sizeof(char));
  
  fd = open("tempTextFile.txt", O_RDONLY);
  if (fd < 0) { perror("r1"); exit(1); }
  
  sz = read(fd, c, 50);
  printf("called read(% d, c, 10).  returned that"
         " %d bytes  were read.\n", fd, sz);
  c[sz] = '\0';
  printf("Those bytes are as follows: % s\n", c);

  int file_desc = open("tempTextFile.txt", O_WRONLY | O_APPEND);
  int copy_desc = dup(file_desc);
  write(copy_desc,"This will be output to the file named dup.txt\n", 46);
  write(copy_desc, "HELLO TEST\n",10);
}
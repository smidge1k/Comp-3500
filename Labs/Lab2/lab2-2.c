#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

int nloop = 50;

/**********************************************************\
 * Function: increment a counter by some amount one by one *
 * argument: ptr (address of the counter), increment       *
 * output  : nothing                                       *
 **********************************************************/
void add_n(int *ptr, int increment){
  int i,j;
  for (i=0; i < increment; i++){
    *ptr = *ptr + 1;
    for (j=0; j < 1000000;j++);
  }
}

int main(){
  int pid;        /* Process ID                     */

  int *countptr;  /* pointer to the counter         */
  bool *interested[2]; /*array of pointers */
  int *turn;  /* pointer to the turn */
  int zero = 0; /* a dummy variable containing 0 */

  int fd;     /* file descriptor to the file "containing" my counter */
  int fd1;    /* file descriptor to the file containing my system variable interested[0]*/
  int fd2;    /* file descriptor to the file containing my system variable interested[1]*/
  int fd3;    /* file descriptor to the file containing my system variable turn*/

  system("rm -f counter");
  system("rm -f interested[0]");
  system("rm -f interested[1]");
  system("rm -f turn");

  /* create a file which will "contain" my shared variable */
  fd = open("counter",O_RDWR | O_CREAT);
  write(fd,&zero,sizeof(int));

  fd1 = open("interested[0]",O_RDWR | O_CREAT);
   write(fd1,&zero,sizeof(bool));

  fd2 = open("interested[1]",O_RDWR | O_CREAT);
   write(fd2,&zero,sizeof(bool));

  fd3 = open("turn",O_RDWR | O_CREAT);
  write(fd3,&zero,sizeof(int));

  /* map files to memory */
  countptr = (int *) mmap(NULL, sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);
  interested[0] = (bool *) mmap(NULL, sizeof(bool),PROT_READ | PROT_WRITE, MAP_SHARED, fd1,0);
  interested[1] = (bool *) mmap(NULL, sizeof(bool),PROT_READ | PROT_WRITE, MAP_SHARED, fd2,0);
  turn = (int *) mmap(NULL, sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd3,0);
  

 
  if (!countptr) {
    printf("Mapping failed\n");
    exit(1);
  }
  *countptr = 0;
  close(fd);

   if (!interested[0]) {
    printf("Mapping failed\n");
    exit(1);
  }
  *interested[0] = false;
  close(fd1);

   if (!interested[1]) {
    printf("Mapping failed\n");
    exit(1);
  }
  *interested[1] = false;
  close(fd2);

  if (!turn) {
    printf("Mapping failed\n");
    exit(1);
  }
  *turn = 0;
  close(fd3);


  setbuf(stdout,NULL);


  pid = fork();
  if (pid < 0){
    printf("Unable to fork a process\n");
    exit(1);
  }

  if (pid == 0) {
    /* The child increments the counter by two's */
    while (*countptr < nloop){
      *interested[0] = true;
      *turn = 1;
      while (*interested[1] && (*turn == 1));
      add_n(countptr,2);
      printf("Child process -->> counter= %d\n",*countptr);
    }
    close(fd);
    *interested[0] = false;
  }
  else {
    /* The parent increments the counter by twenty's */
    while (*countptr < nloop){
      *interested[1] = true;
      *turn = 0;
      while(*interested[0] && (*turn == 0));
      add_n(countptr,20);
      printf("Parent process -->> counter = %d\n",*countptr);
    }
    close(fd);
    *interested[1] = false;
  }
}










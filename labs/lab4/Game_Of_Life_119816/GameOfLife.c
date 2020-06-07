/*
* The Game of Life
*
* Cell is born, if it has exactly three neighbours
* Cell dies of loneliness, if it has less than two neighbours
* Cell dies of overcrowding, if it has more than three neighbours
* Cell survives to the next generation, if it does not die of loneliness or overcrowding
*
* In this version, a 2D array of ints is used.  A 1 cell is on (alive), a 0 cell is off (dead).
* The game plays a number of steps (given by the input), printing to the screen each time 
* when an evolution is performed.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
// Define the timer function for cross compiling 
// based on the architecture
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

typedef unsigned char cell_t;

int size;

int steps, k;

int num_threads;
// Temp is used to store the pointer value for the 
//swap between the old and new evolution board
cell_t **prev, **next, **tmp;

pthread_barrier_t barrier;


// Creating the board (2D array)
cell_t **
allocate_board ()
{

  cell_t **board = (cell_t **) malloc (sizeof (cell_t *) * size);
  if (board == NULL)
    {
      printf ("Memory not allocated for the BOARD\n");
      exit (0);
    }
  int i;

  for (i = 0; i < size; i++)

    board[i] = (cell_t *) malloc (sizeof (cell_t) * size);
  if (board + i == NULL)
    {
      printf ("Memory not allocated for the CELL\n");
      exit (0);
    }
  return board;

}


// Deallocating the board (freeing the 2D array and the board pointer)
void
free_board (cell_t ** board)
{

  int i;

  // Free each cell
  for (i = 0; i < size; i++)

    free (board[i]);
// Free board
  free (board);

  board = NULL;

}

/*
// Custom timer function based on the clock (takt) difference
void
sleep (unsigned int mill)
{
  clock_t start = clock ();
  printf("Clock value:%ld\n",start);
  while ((clock () - start) < mill)
    {
        printf("Clock value:%ld\n",(clock () - start));
    }
}
*/


// Print the board 
void
print (cell_t ** board)
{

  int i, j;

// For each row 
  for (i = 0; i < size; i++)
    {

// Print each column position
      for (j = 0; j < size; j++)

	printf (board[j][i] ? "\033[07m  \033[m" : "  ");
// If the oputput isnt good try this  
//      printf (board[j][i] ? "@" : "  ");

// Followed by a carriage return 
      printf ("\n");
    }

}



// Return the number of "on" (live) cells adjacent to the [i,j] cell 
int
adjacent_to (cell_t ** board, int i, int j)
{

// Adjesent cells counted for a bordered board (board has borders)

  int k, l, count = 0;


  int sk = (i > 0) ? i - 1 : i;

  int ek = (i + 1 < size) ? i + 1 : i;

  int sl = (j > 0) ? j - 1 : j;

  int el = (j + 1 < size) ? j + 1 : j;


  for (k = sk; k <= ek; k++)

    for (l = sl; l <= el; l++)

      count += board[k][l];
// Subtracting the current cell 
  count -= board[i][j];

/*
// This is to count adjesent cells of a bordeles board 
// (adjesent cells od the right border 
// are the cells on the left border and vice versa) same goes for the top
// and bottom

for(int k=-1;k<2;k++){
    for(int l=-1;l<2;l++){
        int col=(i+k+size)%size;
        int row=(j+l+size)%size;
        count+=board[col][row];
    }
}
    count-=board[i][j];
 */

  return count;

}

// Create the board and randomly set values in the  board 
void
create_board (cell_t ** board)
{

  srand ((unsigned int) time (NULL));


  int i, j, k;

  for (i = 0; i < size; i++)
    {

      for (j = 0; j < size; j++)
	{
	  // To randomly populate the grid, we randomly compute
	  // k, which will be either 0, 1, or 2. If k=0, we place
	  // a 1 in board[i][j]. Otherwise, we place a 0. Theoretically,
	  // our grid should be 1/3 1's and 2/3 0's.

	  k = rand () % 3;

	  if (k == 0)
	    board[i][j] = 1;

	  else
	    board[i][j] = 0;

	}

    }

}


// This is the main function that implements the game rules
void
play (int i, int j, int thread_id)
{

// For each cell, apply the rules of Life 
  int a;

  while (k < steps)
    {
      a = adjacent_to (prev, i, j);

      if (a == 2)
	next[i][j] = prev[i][j];

      if (a == 3)
	next[i][j] = 1;

      if (a < 2)
	next[i][j] = 0;

      if (a > 3)
	next[i][j] = 0;

// Barrier for all threads to finish execution
      pthread_barrier_wait (&barrier);


// One thread performs the final execution
      if (thread_id == 0)
	{
	  printf ("%d ----------\n", k + 1);
	  print (next);
	  sleep (1);
	  printf ("\e[1;1H\e[2J");

	  tmp = next;

	  next = prev;

	  prev = tmp;

	  k++;

	}


// Barrier to wait fot the end of the step
      pthread_barrier_wait (&barrier);


    }


  pthread_exit (NULL);

}


// Calculate cell position based on the cell number
void *
defineWork (void *arg)
{


  int thread_id = *((int *) arg);
  int this_x = thread_id / size;
  int this_y = thread_id % size;

  play (this_x, this_y, thread_id);
}

//main----------------------

int
main (int argc, char **argv)
{

  int errCode;
  int snooze;
  printf ("Enter the amount of time the system will sleep (seconds):");
  scanf ("%d", &snooze);
  sleep (snooze);
  printf ("Enter the size of the matrix/board:\n");
  scanf ("%d", &size);
  printf ("Enter the number of evolutions:\n");
  scanf ("%d", &steps);

  prev = allocate_board ();

  create_board (prev);

  next = allocate_board ();

// Print the created board 

  printf ("Initial:\n");

  print (prev);
// How lont the printed board will be displayed
  sleep (3);
  printf ("\e[1;1H\e[2J");

// Number of threads that will be created
  num_threads = size * size;

// Inicializar threads creating array of threads and barrier
  pthread_barrier_init (&barrier, NULL, num_threads);

  pthread_t threads[num_threads];

// Number of evolutions  
  int k = 0;


// Allocate the id number and creating the trhead array
  int i;
  for ( i = 0; i < num_threads; ++i)

    {

      int *arg = malloc (sizeof (int));
      if (arg == NULL)
	{
	  printf ("Memory not allocated for the arg value\n");
	  exit (0);
	}
      *arg = i;

      if ((errCode =
	   pthread_create (&threads[i], NULL, defineWork, arg)) != 0)
	printf ("Error creating thread, error=%d\n", errCode);
    }


// Wait for all threads to finish work
  for (i = 0; i < num_threads; ++i)

    {

      errCode = pthread_join (threads[i], NULL);
      if (errCode != 0)
	printf ("Error joining thread , error=%d\n", errCode);

    }

// Print the last evolution 
  printf ("Final:\n");

  print (prev);

// Destroy the barrier and free the board

  pthread_barrier_destroy (&barrier);

  free_board (prev);

  free_board (next);

}



/*
* Programmed by Tristan Chilvers
* ID: 2288893
* Email: chilvers@chapman.edu
*
*	References:
*
*   ALGORITHMS (used to conceptually understand algorithms and test with their data set):
*   - https://www.geeksforgeeks.org/program-for-sstf-disk-scheduling-algorithm/
*   - https://www.geeksforgeeks.org/scan-elevator-disk-scheduling-algorithms/
*   - https://www.geeksforgeeks.org/c-scan-disk-scheduling-algorithm/
*   - https://www.geeksforgeeks.org/look-disk-scheduling-algorithm/
*
*   FILE:
*   - https://www.tutorialspoint.com/c_standard_library/c_function_fgets.htm
*   - https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
*/

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

//  ============================================================================
//  CONSTANTS
#define SIZE 1000  //  size of service disk
#define CYL_SIZE 5000  //  size of cylinder value

//  ============================================================================
//  GLOBAL VARIABLES
int req[SIZE]; //  the array of requests
int reqCopy[SIZE];  //  copied array of requests
int head; //  head location on service disk
int totCount; // total count of head movements
FILE* file; // cylinder request file pointer
char buffer[SIZE];  // buffer to read each line from the address file

//  ============================================================================
//  FUNCTION HEADERS
int FCFS(); //  First Come First Serve
int SSTF(); //  Shortest Seek Time First
int SCAN(); //  SCAN algorithm, aka the Elevator Algorithm
int CSCAN(); //  CSCAN algorithm, aka the Circular Algorithm
int LOOK(); //  LOOK algorithm
int CLOOK(); //  C-LOOK algorithm

//  Helper Functions -----------------------------------------------------------
void bubbleSort(int n);

//  ============================================================================
//  MAIN PROGRAM
int main(int argc, char const *argv[])
{
  //  ==========================================================================
  //  ERROR CHECK

  //  Not enough arguments
  if(argc != 3)
  {
    printf("Please include the head position and cylinder request file respectfully as an argument.\n");
    exit(EXIT_FAILURE);
  }

  //  Check if backing store file can be opened
  if((head = atoi(argv[1])) < 0)
  {
    printf("Head must be positive integer.\n");
    exit(EXIT_FAILURE);
  }

  //  Check if address file can be opened
  if((file = fopen(argv[2], "r")) == NULL)
  {
    printf("Unable to open cylinder file.\n");
    exit(EXIT_FAILURE);
  }

  //  ==========================================================================
  //  READ FILE AND INSTANTIATE ARRAY
  int index = 0;
  while(fgets(buffer, SIZE, file) != NULL)
  {
    req[index] = atoi(buffer);
    index++;
  }

  //  copy data from original req array to one that can be manipulated for
  //  specific algorithms
  memcpy(reqCopy, req, sizeof(req));

  //  ==========================================================================
  //  RUN EACH ALGORITHM
  printf("Total Head Movement for FCFS:\t%08d\n", FCFS());
  printf("Total Head Movement for SSTF:\t%08d\n", SSTF());
  printf("Total Head Movement for SCAN:\t%08d\n", SCAN());
  printf("Total Head Movement for C-SCAN:\t%08d\n", CSCAN());
  printf("Total Head Movement for LOOK:\t%08d\n", LOOK());
  printf("Total Head Movement for C-LOOK:\t%08d\n", CLOOK());

  fclose(file);

  return 0;
}


//	============================================================================
//	FCFS()
//
//	Input: None. Global variables only.
//	Output:	Using the FCFS scheduling algorithm, find the total amount of
//          head movement
//
int FCFS()
{
  totCount = 0; //  total count of head movements
  int count = 0;  //  difference between head and new position
  int i;  //  used for for loops
  int h = head; // copy head variable for this function

  //  Calculate the difference between each head and positio,
  //  then sum them all
  for(i = 0; i < SIZE; i++)
  {
    count = abs(req[i] - h);
    totCount += count;
    h = req[i];
  }

  return totCount;
}


//	============================================================================
//	SSTF()
//
//	Input: None. Global variables only.
//	Output:	Using the SSTF scheduling algorithm, find the total amount of
//          head movement
//
int SSTF()
{
  int passed[SIZE]; //  Array of passed variables
  int distance[SIZE]; //  Array of the calculated distances
  totCount = 0; //  total count of head movements
  int h = head; //  copy head for function
  int count = 0;  //  difference between head and new position
  int i, j; //  used for for loops
  int pos;  //  the next position for the scheduler
  int skip = 0; //  "boolean" to allow skipping within function
  int len = 0;  //  number of passed variables

  //  instantiate each array element to -1
  for(i = 0; i < SIZE; i++)
  {
    passed[i] = -1;
  }

  //  loop until all requests have been passed
  while(len < SIZE)
  {
    int dist = CYL_SIZE + 1;

    //  find all the distances
    for(i = 0; i < SIZE; i++)
      distance[i] = abs(req[i] - h);

    //  find the shortest distance
    for(i = 0; i < SIZE; i++)
    {
      //  check if request has already been passed
      for(j = 0; j < SIZE; j++)
      {
        if (req[i] == passed[j])
        {
          skip = 1;
          break;
        }
      }

      //  skip if request has already been passed
      if(skip == 1)
      {
        skip = 0;
        continue;
      }

      //  Skip if the head is also the request
      if(h == req[i])
        continue;

      //  grab the closest request
      if(distance[i] < dist)
      {
        dist = distance[i];
        pos = i;
      }
    }

    //  increment values for total and change head
    totCount += dist;
    h = req[pos];
    passed[len] = req[pos];
    len++;
  }

  return totCount;
}

//	============================================================================
//	SCAN()
//
//	Input: None. Global variables only.
//	Output:	Using the SCAN scheduling algorithm, find the total amount of
//          head movements
//
//  *Uses sorting helper method
//
int SCAN()
{
  totCount = 0; //  total count of head movements
  int count = 0;  //  difference between head and new position
  int i;  //  used for for loops
  int h = head; // copy head variable for this function
  int dir = 0; // direction: left is 0, right is 1

  // sort the copied request list
  bubbleSort(SIZE);

  int pos = 0, val = 0;
  for(i = 0; i < SIZE; i++)
  {
    int temp = reqCopy[i];

    if(temp > h)
      break;

    //  if temp is greater
    else if(temp > val)
    {
      pos = i;
      val = temp;
    }
  }

  //  Calculate the difference between each head and position going left,
  //  then sum them all up.
  for(i = pos; i >= 0; i--)
  {
    count = abs(reqCopy[i] - h);
    totCount += count;
    h = reqCopy[i];
  }

  totCount += reqCopy[0];
  totCount += reqCopy[pos];
  h = reqCopy[pos];

  //  Calculate the difference between each head and position going right,
  //  then sum them all up.
  for(i = pos + 1; i < SIZE; i++)
  {
    count = abs(reqCopy[i] - h);
    totCount += count;
    h = reqCopy[i];
  }

  return totCount;
}

//	============================================================================
//	C-SCAN()
//
//	Input: None. Global variables only.
//	Output:	Using the C-SCAN scheduling algorithm, find the total amount of
//          head movements
//
//  *Uses sorting helper method
//
int CSCAN()
{
  totCount = 0; //  total count of head movements
  int count = 0;  //  difference between head and new position
  int i;  //  used for for loops
  int h = head; // copy head variable for this function

  // sort the copied request list
  bubbleSort(SIZE);

  int pos = 0, val = CYL_SIZE + 1;
  for(i = SIZE - 1; i >= 0; i--)
  {
    int temp = reqCopy[i];

    if(temp < h)
      break;

    //  if temp is greater
    else if(temp < val)
    {
      pos = i;
      val = temp;
    }
  }

  //  Calculate the difference between each head and position going right,
  //  then sum them all up.
  for(i = pos; i < SIZE; i++)
  {
    count = abs(reqCopy[i] - h);
    totCount += count;
    h = reqCopy[i];
  }

  totCount += abs((CYL_SIZE - 1) - h);
  h = 0;

  //  Calculate the difference between each head and position going left,
  //  then sum them all up.
  for(i = 0; i < pos; i++)
  {
    count = abs(reqCopy[i] - h);
    totCount += count;
    h = reqCopy[i];
  }

  return totCount + 1; // off by 1 value
}

//	============================================================================
//	LOOK()
//
//	Input: None. Global variables only.
//	Output:	Using the LOOK scheduling algorithm, find the total amount of
//          head movements
//
//  *Uses sorting helper method
//
int LOOK()
{
  totCount = 0; //  total count of head movements
  int count = 0;  //  difference between head and new position
  int i;  //  used for for loops
  int h = head; // copy head variable for this function
  int dir = 0; // direction: left is 0, right is 1

  // sort the copied request list
  bubbleSort(SIZE);

  //  find the first position closest to head
  int pos = 0, val = 0;
  for(i = 0; i < SIZE; i++)
  {
    int temp = reqCopy[i];

    if(temp > h)
      break;

    //  if temp is greater
    else if(temp > val)
    {
      pos = i;
      val = temp;
    }
  }

  //  Calculate the difference between each head and position going left,
  //  then sum them all up.
  for(i = pos + 1; i < SIZE; i++)
  {
    count = abs(reqCopy[i] - h);
    totCount += count;
    h = reqCopy[i];
  }

  //  Calculate the difference between each head and position going right,
  //  then sum them all up.
  for(i = pos - 1; i >= 0; i--)
  {
    count = abs(reqCopy[i] - h);
    totCount += count;
    h = reqCopy[i];
  }

  return totCount;
}

//	============================================================================
//	C-LOOK()
//
//	Input: None. Global variables only.
//	Output:	Using the C-LOOK scheduling algorithm, find the total amount of
//          head movements
//
//  *Uses sorting helper method
//
int CLOOK()
{
  totCount = 0; //  total count of head movements
  int count = 0;  //  difference between head and new position
  int i;  //  used for for loops
  int h = head; // copy head variable for this function
  int dir = 0; // direction: left is 0, right is 1

  // sort the copied request list
  bubbleSort(SIZE);

  //  find the first position closest to head
  int pos = 0, val = 0;
  for(i = 0; i < SIZE; i++)
  {
    int temp = reqCopy[i];

    if(temp > h)
      break;

    //  if temp is greater
    else if(temp > val)
    {
      pos = i;
      val = temp;
    }
  }

  //  Calculate the difference between each head and position going left,
  //  then sum them all up.
  for(i = pos + 1; i < SIZE; i++)
  {
    count = abs(reqCopy[i] - h);
    totCount += count;
    h = reqCopy[i];
  }

  //  Calculate the difference between each head and position going right,
  //  then sum them all up.
  for(i = 0; i <= pos; i++)
  {
    count = abs(reqCopy[i] - h);
    totCount += count;
    h = reqCopy[i];
  }

  return totCount;
}

//  ============================================================================
//  HELPER METHODS
//	----------------------------------------------------------------------------
//	geeksforgeeks.com
//	Recursive Bubble Sorting Algorithm
void bubbleSort(int n)
{
    // Base case
    if (n == 1)
        return;

    // One pass of bubble sort. After
    // this pass, the largest element
    // is moved (or bubbled) to end.
    int i;
    for (i = 0; i < n - 1; i++)
		{
      if (reqCopy[i] > reqCopy[i+1])
			{
				int temp = reqCopy[i];
				reqCopy[i] = reqCopy[i+1];
				reqCopy[i+1] = temp;
			}
		}

    // Largest element is fixed,
    // recur for remaining array
    bubbleSort(n-1);
}

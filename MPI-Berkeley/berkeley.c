//----------------------------------------------------------------------
// Akhil Puvvada 
//
// Compile : mpicc -o berkeley berkeley.c
// Execute : mpirun -np 4 berkeley testfile.txt
// Advanced Operating Systems- Project 2 -Berkeley
//----------------------------------------------------------------------
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define BUF 128 
#define TOT 100
#include <mpi.h>
//----------------------------------------------------------------------
//					Main Function
//----------------------------------------------------------------------
int main(int argc, char *argv[]) 
{
	int size, rank, count;
	int d[10];
	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    char line[TOT][BUF];
    FILE *plist = NULL; 
    int i = 0;
	int j=0;
	int k=0;
	int r=0;
	int diff,ignore;
    int total = 0;
	char *x[100];
	char *token;
	int numVal[100];
	int avg=0;
	int finale;
	
	char const* const file = argv[1];			//read arguments from command line
    plist = fopen(file, "r");		 			//Open the file, File pointer is plist
	//Read the lines from the file
    while(fgets(line[i], BUF, plist)) 
	{
        i++;
    }
	
    total = i;
	
	// calculating the no. of processors from no. of lines(can also be done by using Comm_size) and evaluating
	
	if(size==total-2)
	{
    for(i = 0; i < total; i++)
	{
			
			token = strtok (line[i], ":");		//Split the time into hours, minutes and seconds separated by ":"
			
			 while(token!=NULL)
			 {
			 numVal[j++] = atoi(token);			//numVal array stores all the hours,minutes and seconds in integer format
			 token = strtok (NULL, ":");
			 }	
			 
	}
	
	
	//Struct  to store the hours, minutes and seconds of the time 
	struct {
			int hours[3];
			int min[3];
			int sec[3];
			} time;
			int blocklengths[3]={3,3,3};         //Array of blocklengths
			MPI_Datatype types[3]={MPI_INT,MPI_INT,MPI_INT}; 	//Array of Datatypes
			MPI_Aint displac[3];							//Array of displacements
			MPI_Datatype restype;							//New Datatype
			MPI_Aint intex; 								//Extent
			MPI_Aint intex2;
			MPI_Request request;
			MPI_Type_extent(MPI_INT, &intex);
			MPI_Type_extent(MPI_INT, &intex2);
			displac[0] = (MPI_Aint) 0; 
			displac[1] = intex;
			displac[2] = intex+intex2*3;
			MPI_Type_struct(3, blocklengths, displac, types, &restype); 
			MPI_Type_commit(&restype); 
	
	
     if(rank==numVal[k])   						// Rank as Coordinator
	{
		time.hours[0]=numVal[3*rank+1];			//Assigns values to struct by reading its time by line
		time.min[0]=numVal[3*rank+2];
		time.sec[0]=numVal[3*rank+3];
	}
  
		MPI_Bcast(&time,1,restype,numVal[k],MPI_COMM_WORLD); //Broadcasts coordinator time to all other processes.
		
	if(rank==numVal[k])						//Coordinator process enters here
	{
		printf("\nI am the process with rank %d acting as the coordinator\n",rank);
		printf("Processor %d broadcast it's time %d:%d:%d\n",rank,time.hours[0],time.min[0],time.sec[0]);
		printf( "Coordinator process is sending time to %d\n", rank); 
	}	 
		 sleep(1); // to print cordinator process first
		 printf( "Process %d has received time %d:%d:%d\n", rank,time.hours[0],time.min[0],time.sec[0]); 
		 //rest of the processes read their times with respect to the lines
		 time.hours[1]=numVal[3*rank+1];
		 time.min[1]=numVal[3*rank+2];
		 time.sec[1]=numVal[3*rank+3];
		 //Calculates the difference between it's time and coordinator's time
		 diff=((time.hours[1]*3600)+(time.min[1]*60)+(time.sec[1]))-((time.hours[0]*3600)+(time.min[0]*60)+(time.sec[0]));
		 //sends the differential time to the coordinator
		 MPI_Send(&diff,1,MPI_INT,numVal[k],99,MPI_COMM_WORLD);
		 printf( "Process %d is sending time differential value of %d seconds to process %d\n", rank, diff, numVal[k]); 
	if (rank == numVal[k])
	{
		 // The array's last element is accessed to get the last line 
		 // This is done to ignore the time in the last line
			time.hours[2]=numVal[j-3];
			time.min[2]=numVal[j-2]; 
			time.sec[2]=numVal[j-1];
			//Add up the hours, minutes and seconds and store in ignore
			ignore=((time.hours[2]*3600)+(time.min[2]*60)+(time.sec[2]));
			// Difference Array 
	int diffR[100];
		while(r<size)
	 {
		 // All the processes receive the time differential in seconds
		MPI_Recv(&diff,1,MPI_INT,r,99,MPI_COMM_WORLD,&status);
		printf( "Process %d has received time differential value of %d seconds\n", rank, diff); 
		
		diffR[r] = diff;		//stores into differential array
		r++; 
		// checks if ignore is less than the absolute of the differential time
		if (ignore < abs(diff))
		{
			//ignoring the time difference from last line
			printf( "Coordinator Process is ignoring time differential value of %d from process %d\n", diff, r); 
			count++;
		}
		else
			avg = avg + diff;		//caculates the average if ignore time is not less
	 }
			avg = avg/(size-count);
			printf("Time differential average is %d\n", avg); 
	 
	  	 r=0;
		 while(r<size)
		 {
			 d[r]=avg-diffR[r]; 
			 MPI_Send(&d[r],1,MPI_INT,r,199,MPI_COMM_WORLD);
			 printf("Coordinator process is sending the clock adjustment value of %d to process %d\n", d[r], r);
			 r++;
		 }
		 
	 }
	 MPI_Recv(&d[rank],1,MPI_INT,numVal[k],199,MPI_COMM_WORLD,&status);
	 printf("Process %d has received the clock adjustment value of %d\n", rank, d[rank]); 
	 finale = (time.hours[1]*3600)+(time.min[1]*60) +(time.sec[1]) + d[rank];
	 printf("\nAdjusted local time at process %d is %d:%d:%d\n\n",rank,finale/3600,(finale%3600)/60,(finale%3600)%60);
	MPI_Finalize(); // remove all resources allocated to MPI
 }
 else
 {
	 if(rank==0)
		{
	 printf("Please specify the size: %d\n",total-2);
	 	return 0;
		}
	 MPI_Finalize();
 }

}	
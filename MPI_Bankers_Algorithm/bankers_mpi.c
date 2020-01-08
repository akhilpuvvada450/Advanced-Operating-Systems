/*PROGRAM DOCUMENTATION
Name: Akhil Puvvada - Spring 2019
FILE NAME: bankers_mpi.c
compile : mpicc -o bankers bankers_mpi.c 
execution :  mpirun -np 6 bankers

 **************My Matrices Display Format****************

		A B C -resources Types (1,2,3)
P	P0  X X X
R	P1  X X X
O	.
C	.
E	.
S	.
S	.	


My program when it doesn't find the file, it terminates(Aborts)
*/
/*---------------------Includes---------------------------*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include<time.h>
#define MAX_INPUT_LINE 512
#define TRUE 1 
#define FALSE 0
/*---------------------Main Method-------------------------*/
int main(int argc, char* argv[]) {

    int no_processes, rank;
	FILE *file = NULL;
	FILE *file1 = NULL;
	char line[MAX_INPUT_LINE];
    char *filename = NULL;
	char buf[8];
	int lines;
	
    // Initialise MPI environment
    MPI_Status stat;

    MPI_Init(&argc,
             &argv);
    MPI_Comm_size(MPI_COMM_WORLD,
                  &no_processes);
    MPI_Comm_rank(MPI_COMM_WORLD,
                  &rank);

	int num_resources = 0;

	int a1;
	int a2;
	int a3;
    // Coordinator Rank 0
  if(rank == 0) {
	snprintf(buf,8,"P_%d.txt",rank); // string name to read the file
	filename=buf;
	printf("Process %d is reading file %s \n",rank,filename); // reading file name

    if (filename == NULL){
        printf("Error: Please Enter the File Name.\n"); // prints this if file name is not given
        MPI_Abort(MPI_COMM_WORLD,11);}

    file = fopen(filename, "r");

    if (file == NULL){
        printf("Error: Unable to open file %s\n", filename); // if file is empty
        MPI_Abort(MPI_COMM_WORLD,11);
		}
	fgets(line, MAX_INPUT_LINE, file);
	// read input line by line
	num_resources = atoi(line); // get number of resource types
	
	int instances[num_resources];
	int Finish[no_processes-1];
	int Work[num_resources];
	for(int i=0; i<no_processes-1 ; i++)
	{	
		Finish[i] = FALSE;
	}
	for (int i = 0; i < num_resources; i++)
    {
        fgets(line, MAX_INPUT_LINE, file);
		
		char* token = strtok(line, " "); // Delimiter is " "(space)
		int resourcetype = atoi(token); 
		token = strtok(NULL, " "); 
		int reqvalue= atoi(token);
        instances[i] = reqvalue; // storing instances value of each resource type
    }
	fclose(file);
	int max[no_processes-1][num_resources]; // max matrix of size n*m
	int hold[no_processes-1][num_resources]; // Hold matrix of size n*m
	int Need[no_processes-1][num_resources];// need matrix of size n*m
	int Max_Buffer[num_resources];
	int Hold_Buffer[num_resources];
	int available[num_resources];// Available matrix of size m
	for(int i=1; i<no_processes; i++){
	MPI_Recv(&Max_Buffer,num_resources,MPI_INT,MPI_ANY_SOURCE, 52, MPI_COMM_WORLD, &stat);
	int max_ary_rank=stat.MPI_SOURCE-1;
	MPI_Recv(&Hold_Buffer,num_resources,MPI_INT,MPI_ANY_SOURCE, 48, MPI_COMM_WORLD, &stat);
	int all_ary_rank=stat.MPI_SOURCE-1;
		for(int j=0; j<num_resources;j++){
		max[max_ary_rank][j]=Max_Buffer[j];
		hold[all_ary_rank][j]=Hold_Buffer[j];
		}
	}
	printf("The coordinator is checking if the state is safe :\n");
	printf("\nMax matrix is \n");
	for(int i=0; i<no_processes-1; i++)
	{
		for(int j=0; j<num_resources;j++)
		{
			printf("%d ",max[i][j]);
			if(j==num_resources-1){
            printf("\n");
         }
		}
	}
	printf("\nAllocation matrix is \n");
	for(int i=0; i<no_processes-1; i++)
	{
		for(int j=0; j<num_resources;j++)
		{
			printf("%d ",hold[i][j]);
			if(j==num_resources-1){
            printf("\n");
         }
		}
	}
	for(int i=0;i<no_processes-1;i++)
		{
			for(int j=0;j<num_resources;j++)
			{
				Need[i][j]=max[i][j]-hold[i][j];
			}
		}
	printf("\nNeed matrix is \n");
	for(int i=0; i<no_processes-1; i++)
	{
		for(int j=0; j<num_resources;j++)
		{
			printf("%d ",Need[i][j]);
			if(j==num_resources-1){
            printf("\n");
         }
		}
	}
	for(int j=0;j<num_resources;j++)
	{
		int sum=0;
		for(int i=0;i<no_processes-1;i++)
		{
			sum=sum+hold[i][j];
		}
		available[j]=instances[j]-sum;
	}
	printf("\nAvailable matrix is \n");
	for(int i=0;i<num_resources;i++)
	{
		printf("%d ",available[i]);
	}
	printf("\n");
	for(int i=0;i<num_resources;i++)
		{
			Work[i]=available[i];
		}
		 
	int s=0;
	int check = 0;
	int allocation_check = 0;
	int safeSequence[no_processes-1];
	int seq=0;
	for (int k = 0; k < no_processes-1; k++)
	{
		allocation_check = 0;
		for(int i=0; i<no_processes-1 ; i++)
		{
				
			if(Finish[i] == FALSE)
			{
				check = FALSE;
					for(int j=0; j<num_resources; j++)
					{
						if(Need[i][j] > Work[j])
						{
							check=TRUE;
							break;
						}
					}
					if(check == FALSE)
					{
						safeSequence[seq++] = i; 
                    for(int p=0; p<num_resources ; p++) 
                        Work[p] += hold[i][p]; 
                    Finish[i] = TRUE;
					s++;
					}
				}	
			}
			
		}	
		if(s==no_processes-1){
		printf("the system is in safe state and the safe sequence is \n\n\t");
			 for(int i=0; i<no_processes-2; i++ ){
				printf("P%d ,",safeSequence[i]+1);
			}
			int fv=safeSequence[no_processes - 2]+1;
			printf(" P%d \n\n",fv ); 
		}
		else{
			printf("\n\n Alert : The system is not SAFE !!!\n ABORTING \n\n\n");
			MPI_Abort(MPI_COMM_WORLD,11);
		}
		for(int i=0; i<no_processes - 1;i++ ){
			int a=safeSequence[i]+1;
			printf("The coordinator is allocating resource ");
			for(int l=0; l<num_resources;l++ ){
			printf("%d(%d), ",l+1,hold[a-1][l]);
			
			}
			printf("to process %d \n",a);
			MPI_Send(&a,1,MPI_INT,a,56,MPI_COMM_WORLD); //allocating resources based on sequence
			MPI_Recv(&a3, 1, MPI_INT, a, 49, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			 for(int l=0; l<num_resources;l++ ){
			available[l]=available[l]+hold[a-1][l];//updating available matrix
			Need[a-1][l]=0; // updating need and allocation matrix
			hold[a-1][l]=0;
			max[a-1][l]=0;
			} 
			printf("\nMax matrix is \n");
	for(int i=0; i<no_processes-1; i++)
	{
		for(int j=0; j<num_resources;j++)
		{
			printf("%d ",max[i][j]);
			if(j==num_resources-1){
            printf("\n");
         }
		}
	}
			printf("\nAllocation matrix is \n");
	for(int i=0; i<no_processes-1; i++)
	{
		for(int j=0; j<num_resources;j++)
		{
			printf("%d ",hold[i][j]);
			if(j==num_resources-1){
            printf("\n");
         }
		}
	}
	printf("\nNeed matrix is \n");
	for(int i=0; i<no_processes-1; i++)
	{
		for(int j=0; j<num_resources;j++)
		{
			printf("%d ",Need[i][j]);
			if(j==num_resources-1){
            printf("\n");
         }
		}
	}
	printf("\nAvailable matrix is \n"); 
	for(int i=0;i<num_resources;i++)
	{
		printf("%d ",available[i]);
	}
	printf("\n");
			
		}
    }

    // Remaining processes 
    else {
	
	
	snprintf(buf,8,"P_%d.txt",rank);
	filename=buf;
	printf("Process %d : is reading file %s \n",rank,filename); // reading file name
	
    if (filename == NULL){
        printf("Error: Please Enter the File Name.\n"); // prints this if file name is not given
        MPI_Abort(MPI_COMM_WORLD,11);}
		
	file1 = fopen(filename, "r"); // opening a file for getting count from the file
		if (file1 == NULL)  {
        printf("Error: Unable to open file %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD,11);}
		while(fgets(line, sizeof(line), file1) != NULL){
        lines++;}
	//printf("number of lines Rank %d is %d",rank,lines);
	fclose(file1);
    file = fopen(filename, "r");

    if (file == NULL){
        printf("Error: Unable to open file %s\n", filename); // if file is empty
        MPI_Abort(MPI_COMM_WORLD,11);
		}
	printf("Process  with Rank %d reading the resource allocation \n",rank);
	int max_need[lines];
	int holding[lines];
	for (int i = 0; i < lines; i++)
    {
        fgets(line, MAX_INPUT_LINE, file);
		
		char* token = strtok(line, " ");
		int resource_type= atoi(token); 
		
		token = strtok(NULL, " ");  
		max_need[i]=atoi(token); // taking max_need
		
		token = strtok(NULL, " ");
		holding[i]=atoi(token); // taking hold
    }
	fclose(file); //closing the file
	printf("Process  with Rank %d sending the resource allocation information to  coordinator \n",rank);
	MPI_Send(&max_need,lines,MPI_INT,0,52,MPI_COMM_WORLD); // sending max_need array to Rank 0
	MPI_Send(&holding,lines,MPI_INT,0,48,MPI_COMM_WORLD);// sending holding array to Rank 0
	
	MPI_Recv(&a1, 1, MPI_INT, 0, 56, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive resources from rank 0
	printf("Process %d has received the resources\n",rank);
	printf("Process %d is using the resources\n",rank);
	srand(time(NULL)+rank);
	int time=(rand() % 6)+5;//random number between 5 and 10
	printf("\n\nSLEEP TIME OF RANK %d IS %d seconds\n\n",rank,time);
	sleep(time);
	MPI_Send(&a2,1,MPI_INT,0,49,MPI_COMM_WORLD); // Release resources from rank 0
	printf("Process %d is releasing the resources\n",rank);
    }
    MPI_Finalize();

    return 0;
}

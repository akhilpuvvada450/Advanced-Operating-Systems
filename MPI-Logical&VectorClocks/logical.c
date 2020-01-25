/*----------------------------------------------------------------------
 Akhil Puvvada 

 Advanced Operating Systems
 Project 3 -logical and vector clocks
 file : logical.c
 Compile command: mpicc logical.c -o logical
 Execute command: mpirun -np 6 logical logical.txt
//----------------------------------------------------------------------*/
// includes 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h> 
#include <mpi.h>
#define BUF 128 
#define TOT 100 

//----------------------------------------------------------------------
//					Main Function
//----------------------------------------------------------------------
int main(int argc, char *argv[]) 
{
	
	struct {									//struct to receive and send all data using single variable
	int exec_event; 							//for exec event 
	int send_event;								//for send event
	int p1; 									//process 1 in send
	int p2; 									//process 2 in send
	int time_1; 
	int time_2;
	char msg[20];
	} a;
	
	
	int size;
	int max;
	int m;
	int rank;
	int count;
	int time=0;
	// MPI
	MPI_Init(&argc,&argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int d=1;	//d is set to 1
	int i = 0,j=0,k,total = 0;
	char *token;//for storing tokens from each line
	int array[500];
    char line[TOT][BUF];//for storing lines
	a.p1 = 0;
	a.p2 = 0;
	
	int blocklengths[7]={1,1,1,1,1,1,20}; 		//Array of blocklengths
	MPI_Datatype types[7]={MPI_INT,MPI_INT,MPI_INT,MPI_INT,MPI_INT,MPI_INT,MPI_CHAR}; //Array of Datatypes
	MPI_Aint displac[7]; //Array of displacements
	MPI_Datatype restype; //New Datatype
	MPI_Aint intex,charex;
	MPI_Request request;
	MPI_Type_extent(MPI_INT, &intex);
	MPI_Type_extent(MPI_CHAR, &charex);
	displac[0] = (MPI_Aint) 0; 
	displac[1] = intex;
	displac[2] = intex+intex;
	displac[3] = intex+intex+intex;
	displac[4] = intex+intex+intex+intex;
	displac[5] = intex+intex+intex+intex+intex;
	displac[6] = intex+intex+intex+intex+intex+intex;
	MPI_Type_struct(7, blocklengths, displac, types, &restype);
	MPI_Type_commit(&restype);
	
	//Initializing the clocks to zero	
	int clocks[size];
	for(i=0;i<size;i++)
	{
	clocks[i]=0;			// All clocks to zero
	}
	
	if(rank==0)				// process 0 enters
	{
		i=0;
		int j;
    FILE *fp = NULL; 		//file pointer
     fp = fopen(argv[1], "r"); // opening the file in read only mode
	 if(fp==0)
	 {
		 printf("File not found! Please check if correct file name is specified\n");
		MPI_Finalize();
		 exit(0);
	 }
	 
    while(fgets(line[i], BUF, fp)) //Get the lines from the file
		{
		if(i==0)		//the first line i.e no of processes
		{
			int np = line[0][0]-'0';
			 if (np != size-1)
			 {
			 printf("No. of process provided are incorrect and is aborting\n\n");
			//return 0;
			 MPI_Abort(MPI_COMM_WORLD,10);
			 }
			
			i++;
		}
		else
		{
			char str1[50];
			char str2[50];
		for (j=0;j<20;j++)
		{
			str1[j] = ' ';	//adding spaces for concatenation
			str2[j] = ' ';
			a.msg[j] = ' ';
		}
		str1[0] = '\0';
		str2[0] = '\0';
		a.msg[0] = '\0';
		token = '\0';
		j=0;
		
		
		token = strtok (line[i], " ");//tokenising the lines by space
		while(token!=NULL)
		{
			if (strcmp(token,"end")==0)//comparing if token is end
			{
				int p=1;
				for(p=1;p<size;p++)
				{
					MPI_Send(&a,1,restype,p,9999,MPI_COMM_WORLD);//sending message to all processes to break
				}
				j=9999;
				break; 
			}
			if (j == 1)
			{
				a.p1 = atoi(token);
				j=0;
			}
			
			if (strcmp(token,"exec")==0)
			{
				//if token is exec assign -1 to exec_event
				j=1;
				a.exec_event = -1;
				a.send_event = 0;
			}
			
			if (strcmp(token,"send")==0)
			{
				//if token is send assign -1 to send_event
				a.send_event = -1;
				a.exec_event = 0;
				j=22;
			} 
			
			else if (j == 22)
			{
				a.p1 = atoi(token);
				j = 23;
			}
			
			else if (j == 23)
			{
				a.p2 = atoi(token);
				j = 24;
			}
			
			else if (j == 24)
			{
				strcpy(str1,token);
				
				strcat(str2,str1);
				
				strcat(str2," ");						
			}
			token = strtok (NULL, " ");//the token moving to next 
		}
		
		if (j == 24)
		{
			
			int lo=0;
				for (lo=1;str2[lo]!='\"';lo++)
					a.msg[lo-1] = str2[lo];
		}	
        i++;
	
	
	if(j!=9999)
	{
		
		MPI_Send(&a,1,restype,a.p1,99,MPI_COMM_WORLD);
		
		if(a.exec_event==-1)//exec event
		{
			MPI_Recv(&a,1,restype,MPI_ANY_SOURCE,999,MPI_COMM_WORLD,&status);//recieve ack from rank p
			clocks[status.MPI_SOURCE]=a.time_1;//assign time of first process to array
		}
			else// for send event
			{
				
				int p1=a.p1;
				int p2=a.p2;
				
				MPI_Recv(&a,1,restype,MPI_ANY_SOURCE,999,MPI_COMM_WORLD,&status);
				// assign the time values of each processor into  array to display all at a time at the end 
				clocks[p1]=a.time_1;
				clocks[p2]=a.time_2;
			}
			
		
	}
	else //when j=9999
	{
		for(m=1;m<size;m++)
			 {
				printf("Logical time at the end of process %d is %d\n",m,clocks[m]);//printing all logical times
			 }
		break;
	}
	}	
    }
    total = i; 
    
	}
	else		//all other processes except 0
	{
		while(1)
		{
			 MPI_Recv(&a,1,restype,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			 int tag=status.MPI_TAG;
			 int source=status.MPI_SOURCE;

			 if(tag==9999)//break all processes
			 {
				break;
			 }
			 else
			{
				 if(a.exec_event==-1)
				{
				 time=time+d;//increment time by d=1
				 a.time_1=time;
				 printf( "Execution event in process %d\n", a.p1 );
				 MPI_Send(&a,1,restype,0,999,MPI_COMM_WORLD);
				}
				else if(a.send_event==-1)
				{
					 time=time+d;
					 a.time_1=time;
					 printf( "Send event in process %d\n", a.p1 );
					 a.send_event = -2;
					 MPI_Send(&a,1,restype,a.p2,111,MPI_COMM_WORLD); //tag -111
					 printf( "Message sent from process %d to process %d: %s\n", a.p1, a.p2, a.msg );
				}
				else if (a.send_event == -2)
				{
				   if(time>a.time_1)
				   {
					   max=time;
				   }
				   else
				   {
					   max=a.time_1;
				   }
				   a.time_2=max+1;		//add d to maximum value
				   time=a.time_2;		//max time is assigned to local time of each processor
					printf( "Message received from process %d by process %d: %s\n", status.MPI_SOURCE, rank, a.msg );
					MPI_Send(&a,1,restype,0,999,MPI_COMM_WORLD); // Tag 999
					
				} 
		    }	
		}		
	}
	MPI_Finalize();	
}
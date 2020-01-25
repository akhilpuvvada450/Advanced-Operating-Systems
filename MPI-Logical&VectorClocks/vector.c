/*----------------------------------------------------------------------
 Akhil Puvvada 

 Advanced Operating Systems 
 Project 3 - logical and vector clocks
 file : vector.c
 Compile command: mpicc vector.c -o vector
 Execute command: mpirun -np 4 vector vector.txt
//----------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#define BUF 128 
#define TOT 100 

int main(int argc, char *argv[]) 
{
	
	int size,max,m,l,n;
	int rank;
	int count;
	int time[size];
	
	MPI_Init(&argc,&argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	struct {				//struct to receive and send all data using single variable
	int exec_event;													//for exec event
	int send_event;													//for send event
	int p1;															//process 1 in send
	int p2;															//process 2 in send
	int time_1[size];
	int time_2[size];
	char msg[20];
	} a;
	int i = 0, j=0,k;
	int d=1;//d is set to 1
    int total = 0;
	char *token;//for storing tokens from each line
	int array[500];
    char line[TOT][BUF];//for storing lines
	a.p1 = 0;
	a.p2 = 0;
	
	int blocklengths[7]={1,1,1,1,size,size,20};			//Array of blocklengths
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
	displac[5] = intex+intex+intex+intex+intex*size;
	displac[6] = intex+intex+intex+intex+intex*size+intex*size;
	MPI_Type_struct(7, blocklengths, displac, types, &restype);
	MPI_Type_commit(&restype);
	
	int clocks[size][size];
	for(i=0;i<size;i++)
	{
		time[i]=0;
		for(l=0;l<size;l++)
		{
		clocks[i][l]=0;
		}
	}
	if(rank==0)//process 0
	{
		i=0;
		int j;
		FILE *fp = NULL; //file pointer
		fp = fopen(argv[1], "r");// opening the file in read mode
		if(fp==0)
		{
			printf("File not found! Please check if file name is specified\n");
			MPI_Finalize();
			exit(0);
		}
		while(fgets(line[i], BUF, fp)) //Get the lines from the file
		{
			
			
			if(i==0) 	//the first line i.e no of processes
			{
				/*nt np = line[0][0]-'0';
				if (np != size-1)
			 {
			 printf("No. of process provided are incorrect and is aborting\n\n");
			//return 0;
			 MPI_Abort(MPI_COMM_WORLD,10);
			 }*/
				i++;
			}
			else
			{
			char str1[50];
			char str2[50];
			for (j=0;j<20;j++)
			{
				str1[j] = ' ';//adding spaces for concatination
				str2[j] = ' ';
				a.msg[j] = ' ';
			}
			str1[0] = '\0';		//adding spaces for concatenation
			str2[0] = '\0';
			a.msg[0] = '\0';
			token = '\0';
			j=0;
			token = strtok (line[i], " ");//tokenize lines according to spaces
			while(token!=NULL)
			{
				if (strcmp(token,"end")==0)//if token is end
				{
					int p=1;
					for(p=1;p<size;p++)
					{
						MPI_Send(&a,1,restype,p,9999,MPI_COMM_WORLD);//sending message to break
					}
					j=9999; 
					break;
				}
				if (j == 1)
				{
					a.p1 = atoi(token);
					j=0;
				}
				if (strcmp(token,"exec")==0)//compare token with exec
				{
					j=1;
					a.exec_event = -1;//set exec_event to -1
					a.send_event = 0;
				}
				if (strcmp(token,"send")==0)//compare token with send
				{
					a.send_event = -1;//set send_event to -1
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
				token = strtok (NULL, " ");//incrementing the token
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
			if(a.exec_event==-1)//for exec event
			{
				MPI_Recv(&a,1,restype,MPI_ANY_SOURCE,999,MPI_COMM_WORLD,&status);
				for(i=0;i<size;i++)
				{
					clocks[status.MPI_SOURCE][i]=a.time_1[i];//update time of processor in clocks 
				}
				
			}
			else//for send event
			{
				int p1=a.p1;
				int p2=a.p2;
				MPI_Recv(&a,1,restype,MPI_ANY_SOURCE,999,MPI_COMM_WORLD,&status);
				for(i=0;i<size;i++)
				{
					clocks[p1][i]=a.time_1[i];//upadte clocks 
					clocks[p2][i]=a.time_2[i];
				}
				}
		}
		else//j=9999 break all processes 
		{
			for(m=1;m<size;m++)
				 {
					 printf("Vector time at process %d is [ ",m);
					 for(n=1;n<size;n++)
					 {
					printf("%d ",clocks[m][n]);//print clock values
					 }
					 printf("]\n");
				 }
			break;
		}
		}
		}
		total = i; 
		
	}
	else//all other processes
	{
		while(1)
		{
		 MPI_Recv(&a,1,restype,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		 int tag=status.MPI_TAG;
		 int source=status.MPI_SOURCE;
		 if(tag==9999)
		 {
			break; // break all processes
		 }
		 else{
			 if(a.exec_event==-1)//for exec
			{
				time[rank]=time[rank]+1;
				for(i=1;i<size;i++)
				{
					a.time_1[i]=time[i];//time of all processes is updated
				}
				printf( "Execution event in process %d\n", a.p1 );
			 MPI_Send(&a,1,restype,0,999,MPI_COMM_WORLD);
			}
		else if(a.send_event==-1)//send event
		{
			time[rank]=time[rank]+1;
			for(i=1;i<size;i++)
			{ 
					a.time_1[i]=time[i];//time of all processes is updated
			}
			printf( "Send event in process %d\n", a.p1 );
			a.send_event = -2;
			MPI_Send(&a,1,restype,a.p2,111,MPI_COMM_WORLD);
			sleep(1);
		}
		else if (a.send_event == -2)//send for second process
		{
			printf( "Message received from process %d by process %d: %s\n", status.MPI_SOURCE, rank, a.msg);
		   if(a.time_1[rank]>time[rank])
			{
				a.time_2[rank]=a.time_1[rank]+1;//update time of second process
			}
			else
			{
				a.time_2[rank]=time[rank]+1;
			}
			for(i=0;i<size;i++)
			{
				if(i!=rank)
				{
				a.time_2[i]=a.time_1[i];	
				}
					if(time[i]<a.time_2[i])
					time[i]=a.time_2[i];				
			}
			MPI_Send(&a,1,restype,0,999,MPI_COMM_WORLD);//completion message to 0
			} 
			}	
			}		
	}
	
	MPI_Finalize();
	
}
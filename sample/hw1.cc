#include <cstdio>
#include <stdlib.h>
#include <mpi.h>
#include <iostream>
#include<boost/sort/spreadsort/spreadsort.hpp>

int has_swap;
void swap(float *a, float *b){
	float t = *a;
	*a = *b;
	*b = t;
	// has_swap = 1;
}

int compare(const void *a, const void *b){
	float num1 = *(float*)a;
	float num2 = *(float*)b;
	if(num1<num2){
		return -1;
	}
	else if(num1>num2){
		return 1;
	}
	else{
		return 0;
	}
}

//simple classic quicksort algorithm
int partition (float arr[], int low, int high) 
{ 
    float pivot = arr[high];
    int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far
  
    for (int j = low; j <= high - 1; j++) 
    { 
        // If current element is smaller than the pivot 
        if (arr[j] < pivot) 
        { 
            i++; // Increment index of smaller element 
            swap(&arr[i], &arr[j]); 
        } 
    }
    swap(&arr[i + 1], &arr[high]); 
    return (i + 1); 
} 

void quickSort(float arr[], int low, int high) 
{ 
    if (low < high) 
    { 
        /* pi is partitioning index, arr[p] is now 
        at right place */
        int pi = partition(arr, low, high); 
  
        // Separately sort elements before 
        // partition and after partition 
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    }
}


int main(int argc, char** argv){
	double Comm_Time,CPU_Time,IO_Time,t_start,t_end,t_temp;
	Comm_Time = CPU_Time = IO_Time = t_start = t_end = 0.0;
	
	
	MPI_Init(&argc, &argv);
	t_start = MPI_Wtime();
	int N = atoi(argv[1]);
	int rank, size;
	int rc, i, j, k, left, right;
	int result = true;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Comm new_comm = MPI_COMM_WORLD;
    MPI_Group orig_group, new_group;
	MPI_Status status;

	// Recount the data_per_proc
	int quotient = N/size;
	int remainder = N%size;
	int n = quotient + (rank<remainder);
	int max_n_per_proc = quotient + 1; //some rank will take 1 more data (n+1)
	int offset = rank*quotient + ((rank<remainder) ? rank:remainder);
	
	int last_n_buf = (rank == 0 ? 0 :  (quotient + (rank-1<remainder)));
	int next_n_buf = (rank == size - 1 ? 0 : (quotient + (rank+1<remainder)));


	float *data = (float*)malloc(n*sizeof(float));		
	float *temp = (float*)malloc(n*sizeof(float));
	float *buf  = (float*)malloc((n+1)*sizeof(float));
	t_end = MPI_Wtime();
	CPU_Time += t_end - t_start;

	t_start = MPI_Wtime();
	// Read input data
	MPI_File fh;
	MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	MPI_File_read_at(fh, offset*sizeof(MPI_FLOAT), data, n, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&fh);
	t_end = MPI_Wtime();
	IO_Time += t_end - t_start;
	// printf("rank %d got float: %f\n", rank, data[0]);	

	//local sort , before goto proc swap
	// quickSort(data, 0, n-1);
	// new way, predefined qsort in C lib is better than quickSort()
	// spreadsort is the most robust sort
	t_start = MPI_Wtime();
	// qsort(data, n, sizeof(float), compare);
	boost::sort::spreadsort::spreadsort(data, data+n);
	t_end = MPI_Wtime();
	CPU_Time += t_end - t_start;

	while(result){
		// printf("CheckCheck\n");
		
		has_swap = 0;
		// ############################################################################
		
		if(rank&1){ // odd index
			if(n != 0){ //some blank proc take no data
				if(last_n_buf != 0){ // double check ,if n==0,there is no last_n
					t_start = MPI_Wtime();
					MPI_Send(data, n, MPI_FLOAT, rank-1, rank, MPI_COMM_WORLD);
					MPI_Recv(buf, last_n_buf, MPI_FLOAT, rank-1, rank-1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					t_end = MPI_Wtime();
					Comm_Time += t_end - t_start;								
					
					t_start = MPI_Wtime();
					i = n-1; left = n-1; right = last_n_buf-1;
					for(;i>=0;i--){
						if(left < 0 || right < 0){
							break;
						}
						if(data[left] >= buf[right]){
							temp[i] = data[left--];
						}
						else{
							temp[i] = buf[right--];
							has_swap = true;
						}
					}

					// for(int idx=0;idx<n;idx++){
					// 	float t = temp[idx];
					// 	temp[idx] = data[idx];
					// 	data[idx] = t;
					// }

					//use pointer instead of variable assign in iteration
					float *t;
					t = data;
					data = temp;
					temp = t;

					t_end = MPI_Wtime();
					CPU_Time += t_end - t_start;
				}
			}
		}
		else{ //even index
			if(n != 0){
				if(next_n_buf != 0){
					t_start = MPI_Wtime();
					MPI_Recv(buf, next_n_buf, MPI_FLOAT, rank+1, rank+1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					MPI_Send(data, n, MPI_FLOAT, rank+1, rank, MPI_COMM_WORLD);
					t_end = MPI_Wtime();
					Comm_Time += t_end - t_start;	
					
					t_start = MPI_Wtime();
					i=0, left = 0, right = 0;
					for(; i<n; i++){
						if(left >= n || right >= next_n_buf){
							break;
						}
						if(data[left] <= buf[right]){
							temp[i] = data[left++];
						}
						else{
							temp[i] = buf[right++];
							has_swap = true;
						}
					}

					for(;i<n;i++){
						temp[i] = data[left++];
					}

					//use pointer instead of variable assign in iteration
					float *t;
					t = data;
					data = temp;
					temp = t;

					t_end = MPI_Wtime();
					CPU_Time += t_end - t_start;
				}
			}
		}
		
		// ############################################################################
		
		if(rank&1){ //odd index
			if(n != 0){
				if(next_n_buf != 0){
					t_start = MPI_Wtime();			
					MPI_Send(data, n, MPI_FLOAT, rank+1, (rank), MPI_COMM_WORLD);
					MPI_Recv(buf, next_n_buf, MPI_FLOAT, rank+1, (rank+1), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					t_end = MPI_Wtime();
					Comm_Time += t_end - t_start;		

					t_start = MPI_Wtime();				
					i=0, left = 0, right = 0;
					for(; i<n; i++){
						if(left >= n || right >= next_n_buf){
							break;
						}
						if(data[left] <= buf[right]){
							temp[i] = data[left++];
						}
						else{
							temp[i] = buf[right++];
							has_swap = true;
						}
					}

					for(;i<n;i++){
						temp[i] = data[left++];
					}

					//use pointer instead of variable assign in iteration
					float *t;
					t = data;
					data = temp;
					temp = t;

					t_end = MPI_Wtime();
					CPU_Time += t_end - t_start;
				}
			}
		}else{ //even index
			if(n != 0){
				if(last_n_buf != 0){
					t_start = MPI_Wtime();			
					MPI_Recv(buf, last_n_buf, MPI_FLOAT, rank-1, (rank-1), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					MPI_Send(data, n, MPI_FLOAT, rank-1, (rank), MPI_COMM_WORLD);
					t_end = MPI_Wtime();
					Comm_Time += t_end - t_start;

					t_start = MPI_Wtime();					
					i = n-1; left = n-1; right = last_n_buf-1;
					for(;i>=0;i--){
						if(left < 0 || right < 0){
							break;
						}
						if(data[left] >= buf[right]){
							temp[i] = data[left--];
						}
						else{
							temp[i] = buf[right--];
							has_swap = true;
						}
					}

					//use pointer instead of variable assign in iteration
					float *t;
					t = data;
					data = temp;
					temp = t;

					t_end = MPI_Wtime();
					CPU_Time += t_end - t_start;
				}
			}
		}
		t_start = MPI_Wtime();	
		MPI_Allreduce(&has_swap, &result, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
		t_end = MPI_Wtime();
		Comm_Time += t_end - t_start;
		
	}
	
	t_start = MPI_Wtime();
	// printf("Myrank:%d, Data:%f\n", rank, data[0]);
	MPI_File_open(MPI_COMM_WORLD, argv[3], MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
	rc = MPI_File_write_at(fh, sizeof(float)*offset, data, n, MPI_FLOAT, MPI_STATUS_IGNORE);
	if(rc != MPI_SUCCESS){
		printf("Not write successful!\n");
	}
	MPI_File_close(&fh);
	t_end = MPI_Wtime();
	IO_Time += t_end - t_start;

	t_start = MPI_Wtime();	
	free(data);
	free(buf);
	free(temp);	
	t_end = MPI_Wtime();
	CPU_Time += t_end - t_start;

	t_temp = IO_Time;
	MPI_Reduce(&t_temp, &IO_Time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	t_temp = Comm_Time;
	MPI_Reduce(&t_temp, &Comm_Time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	t_temp = CPU_Time;
	MPI_Reduce(&t_temp, &CPU_Time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	

	if(rank==0){
		printf("I/O Time:%lf\n", IO_Time);
		printf("Communication Time:%lf\n", Comm_Time);
		printf("Computation Time:%lf\n", CPU_Time);
	}

	MPI_Finalize();
	return 0;
}
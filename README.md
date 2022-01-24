# 2021 Parallel Programming HW1 Odd-Even-Sort

## 1. Implementation
In this homework, the target goal is implement odd even sort algorithm through MPI interface. The whole process can be seperate as 6 parts:
- allocate memory
- read input_data
- MPI process communication
- local sorting
- reduce
- write output_data

### Load Balancing
In the load balancing part, since there always have ```number of data(n)``` greater than ```number of proc```, thus most of the data need handle:
> **n / size**

But some processes need handle:
>  **n / size + 1  if rank < n % size**

### Data Preprocessing
I will prepare three memory space (**data, temp, buf**) for each rank. ```data``` is used t store the current load **input_data** or **sorted_data** from which received from another rank, ```data``` will be output as output value from each rank after all processes had done assigned task. ```temp``` is used for the temp memory space in **merge-sort** phase. ```buf``` is the receive buffer when using MPI_Recv to receive sorted data from other rank.

### Odd-Even Sort Phase

## 2. Optimization

## 3. Experiments

### Strong Scalability

### Time Profile

## 4. Conclusion

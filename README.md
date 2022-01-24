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
Let's explain how the odd-even phase work. In the even stage, there is only the rank which ```rank%2==0```or```n&1``` is true will recv the data from the odd rank (even rank-1) through MPI_Send and MPI_Recv. When they recv data from another rank, they save them in buf memory first, and try to merge the local ```data``` and recv ```buf``` in ```temp```, and send the rest data back after sorting. Odd stage also do the samething, for more details please check my report documents.

### Reduce
The even and odd stage are executed in while loops, it will be break when the **MPI_Reduce** result of ```MPI_LOR``` is false. Because when if there is no any **swap** happened, **has_swap** will be false, which is determined in the **merge-sort** part.

## 2. Optimization
- Use % instead of %
- Avoid to use ternary operator (e.g. a ? b : c)
- Reassign pointer target instead of copy the array value when doing Array memory swap
- ++i > i++ > i+=1 > i=i+1
- Pick a suitable sorting algorithm
- Dynamic allocating, load balancing
- Avoid function memory allocation
- Avoid logical, duplicate redudant computing, use constant value

## 3. Experiments

### Strong Scalability

### Time Profile

## 4. Conclusion

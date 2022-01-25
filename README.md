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
1. Use & instead of %

  Use bit operator instead of arithmetic operator is better for compiler.
  
2. Avoid to use ternary operator (e.g. a ? b : c)

  Ternary operator also is a heavy cost operation, try to avoid use it.
 
3. Reassign pointer target instead of copy the array value when doing Array memory swap

  This is important, relocate array pointer instead of copying array elements one by one, save a lot of time.
  
4. ++i > i++ > i+=1 > i=i+1

  A code writing hint.

5. Pick a suitable sorting algorithm

  In this repo case, for local sort before start odd-even-sort, speadsort > qsort(C Library) > self-defined quicksort. It seems like qsort is optimized in the library, hence becareful when you choose sorting algorithm.

6. Dynamic allocating, load balancing

  Using dynamic way to allocate task job to each MPI rank.

7. Avoid function memory allocation
8. Avoid logical, duplicate redudant computing, use constant value

## 3. Experiments

### System Spec
- 4 Nodes, 12 CPUS ( if 1 cpu for 1 thread, then maximum is 48 threads in a same time )
- OS: Arch Linux
- Compilers: GCC 10.2.0, Clang 11.0.1
- MPI: Intel MPI Library, Version 2019 Update 8
- Scheduler: Slurm 20.02.5
- Network: Infiniband

### Strong Scalability (Speedup Factor)
Different Process Performance        |  Speedup Factor
:-------------------------:|:-------------------------:
<!-- ![](./time/images/33_single_node_diff_proc_bar.png)  |  ![](./time/images/33_single_node_diff_proc_line.png) -->

### Time Profile!


## 4. Conclusion

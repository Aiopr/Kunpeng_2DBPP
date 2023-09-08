# 2BP Summary

From [A goal-driven ruin and recreate heuristic for the 2D variable-sized bin packing problem with guillotine constraints](https://www.sciencedirect.com/science/article/pii/S0377221721009826)

## Problem Variants

### Basic Problem

The **Two-Dimensional Bin Packing Problem (2BP)** consists of packing a heterogeneous set of small rectangular items into larger rectangular bins. 

In general, a 2BP solution is considered **feasible** if

- the items are fully inside the bin
- items are not overlapping 
- the edges of the items are parallel to the edges of the bin

The objective is to **minimize the amount of unused bin area** incurred by packing all the items.

### Variants

- Rotation: $|R|$ (Rotated) if the items are allowed to rotate $90\degree$; $|O|$ (Oriented) otherwise .

- Guillotine: All cuts must be edge-to-edge cuts. Only solutions satisfying the guillotine constraints can be represented by a tree. (see following section)

  ![](https://raw.githubusercontent.com/Aiopr/CloudIMG/main/image-20230331130916149.png)

The algorithm addresses four variants of 2BP:

- **2BP|O|G**: identical bins, no rotation
- **2BP|R|G**: identical bins, $90\degree$ rotation
- **2VSBP|O|G**: heterogeneous set of bins (variable-sized), no rotation
- **2VSBP|R|G**: heterogeneous set of bins (variable-sized), $90\degree$ rotation

## Solution Representation

A complete solutions consists of several cutting patterns. Each cutting pattern is represented as a rooted tree, as the following figure shows. 

- White: structure node (**V** if the cut performed on this node to generate next stage is vertical; **H** otherwise)
- Light Gray: leftover node 
- Dark Gray: item node

<img src="https://raw.githubusercontent.com/Aiopr/CloudIMG/main/image-20230331131839827.png" alt="image-20230331131839827" style="zoom: 67%;" />

Some properties not mentioned in this paper: 

- The order of a node's child nodes doesn't matter. Changing the order does not change the final answer. 
- From the above point, we can establish that among a node's child nodes, there exists one way to merge all the leftover nodes into one node. (这一点他的代码里面没有写，可以做一点小优化)

### Notation 

- A solution $S=\{C,E\}$ not only contains a set of cutting patterns $C$, it also consists of a set of excluded items $E$. 

## Algorithm

### High-level Overview

<img src="https://raw.githubusercontent.com/Aiopr/CloudIMG/main/image-20230331133136729.png" alt="image-20230331133136729" style="zoom: 67%;" />

- **Accept**: If the cost function of the current solution is better than the best solution, then it is accepted. (Notice that the current solution is not necessary a **feasible** solution)

![image-20230331134314546](https://raw.githubusercontent.com/Aiopr/CloudIMG/main/image-20230331134314546.png)

# Homochromatic Square Perimeters

> Cells in an $N$-by-$N$ grid $G$ are colored in monochrome:
> 
> $$\forall~0\leq i,j<N,\text{let}~G_{i,j}=\begin{cases}
> 0&\text{white}\\
> 1&\text{black}
> \end{cases}.$$
> 
> Find the smallest non-negative integer $K$, such that there exists no square of cells in $G$, with side length $K'>K$, whose perimeter consists entirely of black cells.

[Codeforces/Polygon practice problem](http://s.gilgamesh.cc/32).

## $O(N^3)$ via pre-computation

The problem reduces to finding the side length of the largest square whose perimeter is entirely black, or $0$ if no black cells exist. We denote such squares *valid*. Let $L_{i,j}$ store the largest length of consecutive black cells, starting from cell $i,j$ and moving left. For white cell $G_{i,j}=0$, let $R_{i,j}=0$. Similarly, define $U_{i,j}$ for moving upwards. The $O(N^2)$ indices for $L,U$ may be computed in $O(1)$ each with the recurrence

$$L_{i,j}=\begin{cases}
G_{i,j}&j=0\\
0&G_{i,j}=0\\
L_{i,j-1}+1&G_{i,j}=1
\end{cases}$$

and similarly for $U$.

Each square in $G$ may be specified by a tuple $(i,j,k)$ of upper-left corner $i,j$ and side length $k$. The square $Q_{i,j,k}$ is valid if and only if all four edges are entirely black; that is, all four conditions below are satisfied:

$$\begin{cases}
L_{i,j+k-1}\geq k&\text{top edge}\\
U_{i+k-1,j+k-1}\geq k&\text{right edge}\\
L_{i+k-1,j+k-1}\geq k&\text{bottom edge}\\
U_{i+k-1,j}\geq k&\text{left edge}
\end{cases}$$

For each of the $O(N^3)$ squares, this check for validity costs $O(1)$.

[$O(N^3)$ C++ code](homochromatic-square-perimeters.md-assets/solution-n3.cpp).

## $O(N^2\alpha(N))$ amortized via radix sort and DSU/Union-Find

First, compute $L',U'$, similarly to $L,U$, but for lengths of black cells moving right, and downwards, respectively.

Each of the $O(N)$ upper-left-to-bottom-right diagonals may be specified by $(i,j)$ as the diagonal which passes through cell $i,j$. Denote the diagonals by $D$. Note that $D_{i,j}$ specifies the same diagonal as $D_{i+1,j+1}$.

**For a given upper-left corner $i,j$, we check all squares $Q_{i,j,k}$ for validity by querying a DSU for the maximum valid $k$ for that $i,j$.** To begin, define the *reach* $R$ over the diagonal:

$$R_{i,j}=\min(U_{i,j},L_{i,j})$$

and *reverse reach* $R'$:

$$R'_{i,j}=\min(U'_{i,j},L'_{i,j}).$$

Intuitively, the reach $R$ specifies the maximum side length for a valid square with bottom-right corner $i,j$, while the reverse reach $R'$ specifies similarly for the upper-left corner $i,j$. In other words, square $Q_{i,j,k}$ is valid if and only if both conditions below are satisfied:

$$\begin{cases}
R'_{i,j}\geq k\\
R_{i+k-1,j+k-1}\geq k
\end{cases}.\tag{1.1, 1.2}$$

We can remove condition $(1.2)$’s dependence on $k$ by defining the *discounted reach* $R''$:

$$R''_{i,j}=R_{i,j}-i.$$

$(1.1, 1.2)$ may now be rewritten:

$$\begin{cases}
k\leq R'_{i,j}\\
R''_{i+k-1,j+k-1}\geq 1-i
\end{cases}.\tag{2.1, 2.2}$$

We process the upper-left corners $i,j$ along diagonal $D_{i,j}$ in increasing order of $i$, $1-i$ decreases. Thus, the set $S$ of indices $(i',j')=(i+k-1,j+k-1)$ which satisfy $(2.2)$ never shrinks. More specifically, at $i,j$, all bottom-right corners $(i',j')$ with $R''_{i',j'}\geq 1-i$ must be added to $S$. As we have already processed $(i-1,j-1)$ at this point, only the bottom-right corners which satisfy $R''_{i',j'}=1-i+1$ need to be added. We may query for these $(i',j')$ in $O(1)$ by pre-sorting in $O(N)$, with radix sort, all the $R''_{i',j'}$ on the diagonal.

At a given upper-left corner $i,j$, then, the maximal valid bottom-right corner is the maximal $(i',j')\in S$ which also satisfies $(2.1)$:

$$k=i'-i+1\leq R'_{i,j}\implies i'\leq R'_{i,j}+i-1.\tag{3}$$

Formally, we wish to make two types of queries to set $S$:

1. $Add(i,j)$: Add $(i,j)$ to the set.
2. $Get(i')$: Query the largest $(i,j)$ in the set with $i\leq i'$.

Observe that for some $i,j$, if $(i,j)\notin S$, then $Get(i)=Get(i-1)$. This leads us to consider implementing $S$ with a DSU. Should we process upper-left corners in decreasing order of $i$ instead of increasing order, $1-i$ increases, and $S$ never grows. Thus, instead of $Add$, we wish to support $Remove$.

Before moving on to the reduction to DSU, we first discuss an example.

> Take, for example, the following input $G$ with $K=4$ from the square with upper-left corner at $(1, 1)$:
> 
> ```
> 110111
> 011110
> 010011
> 111011
> 111111
> 111111
> ```
> 
> Consider the processing of the major diagonal $D_{0, 0}$. As mentioned, we step through the diagonal in decreasing order of $i$.
> 
> | $i$ | $j$ | $R$ | $R'$ | $R''$ | $R'+i-1$ | $1-i$ |
> | --- | --- | --- | ---- | ----- | -------- | ----- |
> | $5$ | $5$ | $4$ | $1$  | $-1$  | $5$      | $-4$  |
> | $4$ | $4$ | $5$ | $2$  | $1$   | $5$      | $-3$  |
> | $3$ | $3$ | $0$ | $0$  | $-3$  | $2$      | $-2$  |
> | $2$ | $2$ | $0$ | $0$  | $-2$  | $1$      | $-1$  |
> | $1$ | $1$ | $1$ | $4$  | $0$   | $4$      | $0$   |
> | $0$ | $0$ | $1$ | $1$  | $1$   | $0$      | $1$   |
> 
> Observe, as noted previously, that $R'+i-1$ intuitively calculates the maximum row index which may serve as a bottom-right corner for a given upper-left corner $i,j$. In addition, for any upper-left corner $i,j$, it may form a valid square with bottom-right corner $i',j'$ as long as $R''_{i',j'}\geq 1-i$ and $R'_{i,j}+i-1\geq i'$. These together describe conditions $(2.2)$ and $(3)$, respectively.
> 
> To visualize the operations on $S$, we arrange it onto a line with corner $(0,0)$ on the left and $(5,5)$ on the right. Initially, all corners are part of $S$:
> 
> ```
> ||||||
> 012345
> ```
> 
> At any point in time, we either remove an element from $S$, or we stand at some point on the line, and look left to identify the index of the first `|` blocking our view. Since $S$ is unchanged prior to visiting $(3,3)$, we take that as our first example. As $1-i$ is $-2$, we must remove all $(i',j')$ from $S$ where $R''_{i',j'}$ is smaller than $-2$. Thus, we remove $(3,3)$ from $S$:
> 
> ```
> |||_||
> 012345
> ```
> 
> Intuitively, $(3,3)$ has been removed from consideration as a bottom-right corner. This is because any bottom or right edges which extend from it can no longer reach any upper-left corners we have yet to process, or the upper-left corner we are currently processing: $(3,3)$.
> 
> We wrap up processing upper-left corner $(3,3)$ by querying $S$ for the maximal bottom-right corner with $i'\leq R'_{3,3}+3-1=2$, which is the bottom-right corner $(2,2)$. To visualize this, imagine standing at index $2$ on the $S$ line and looking left for the first `|` blocking your vision (index $2$ counts). This is the $|$ at $2$, which represents bottom-right corner $(2,2)$.
> 
> ```
> |||_||
> 012345
>   ^
> ```
> 
> This means that the maximal valid square with upper-left corner at $(3,3)$ has bottom-right corner at $(2,2)$, which is not a valid square at all. This is expected, since there is no valid square with upper-left corner at $(3,3)$, as the cell is colored white.
> 
> In the next step, we process upper-left corner $(2,2)$. Noting first that its $1-i$ is $-1$, we must remove all indices $(i',j')$ from $S$ which have $R''_{i',j'}<-1$. Once again, that bottom-right corner is $(2,2)$:
> 
> ```
> ||__||
> 012345
>  ^
> ```
> 
> and when we eventually query, we will stand at index $R'_{2,2}+2-1=1$ and look left, to which we see the bottom-right corner $(1,1)$. This once again means that no valid square exists with upper-left corner $(2,2)$.
> 
> In the next step, we process upper-left corner $(1,1)$. We have its $1-i=0$, we we will remove from $S$ the indices where $R''_{i',j'}=-1$ now. This turns out to be bottom-right corner $(5,5)$:
> 
> ```
> ||__|_
> 012345
> ```
> 
> Now, we stand at index $R'_{1,1}+1-1=4$ and look left:
> 
> ```
> ||__|_
> 012345
>     ^
> ```
> 
> Luckily, we see the bottom-right corner $(4,4)$, which means that, as expected, the maximal valid square with upper-left corner at $(1,1)$ has side length $4-1+1=4$. We must now update our answer if necessary.
> 
> In the final step, we process upper-left corner $(0,0)$. $1-i=1$, so we remove any indices where $R''=0$. The only such index in $S$ is $(1,1)$:
> 
> ```
> |___|_
> 012345
> ```
> 
> Standing at index $R'+0-1=0$ and looking left, we see $(0,0)$, which means that the maximal valid square starting at $(0,0)$ has side length $1$.

We reduce $S$ into a DSU $S'$ by implementing it together with array $S''$, both over indices $(i,j)$ on $D_{i,j}$. Intuitively, $S'$ stores which $Get$s return the same answer, and $S''$ stores that answer. Initialize each $(i,j)$ of $S'$ as disjoint, and each $S''_{i,j}=(i,j)$.

1. $Remove(i,j)$: Now, $Get(i)$ must give the same answer as $g=Get(i-1)$, so update $S'$ with $Union((i,j), (i-1,j-1))$. Then update $S''$:
   
	 $$S''_{Find(i,j)}=g.$$
2. $Get(i')$: Return $S''_{Find(i',j')}$ for the $j'$ on the diagonal.

For each of the $O(N)$ diagonals, $S$ contains $O(N)$ elements and is queried $O(N)$ times. These queries are bottlednecked by the $S'$ DSU implementation, costing $O(N\alpha(N))$ per diagonal. For the implementation, care must be taken with $Remove(i,j)$ for the smallest $i$ in a diagonal.

[$O(N^2\alpha(N))$ C++ code](homochromatic-square-perimeters.md-assets/solution-ackermann.cpp).

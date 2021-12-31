# Homochromatic Square Frames

> Each cell in a $N$-by-$N$ grid $G$ is colored either black or white:
> 
> $$G_{i,j}=\begin{cases}
> 0&\text{white}\\
> 1&\text{black}
> \end{cases}.$$
> 
> Find the side length $Z$ of the largest square of cells, such that all the cells on the boundary of the square are black.

Select solutions are presented below in decreasing order of runtime complexity. Any additional improvements are welcome.

## $O(N^3)$

Let $U_{i,j}$ store the largest length of consecutive black cells, starting from cell $i,j$ and moving upwards. Similarly, define $R_{i,j},D_{i,j},L_{i,j}$. One can then iterate over all possible upper-left corners of squares $i,j$, and for each possible square size $k$, check that we have the required lengths of consecutive black cells on each edge:

```
Z = 0
for i in [0, N]:
	for j in [0, N]:
		for k in [1, N]:
			# Range check.
			if i + k - 1 >= N or j + k - 1 >= N:
				continue
			# Top & left edges must be fully black.
			if R[i][j] >= k and D[i][j] >= k
				# Bottom & right edges must be fully black.
				and U[i + k][j + k] >= k
				and L[i + k][j + k] >= k:
				Z = max(Z, k)
```

What remains is to compute $U,R,D,L$. We demonstrate the computation for $U$; the other three matrices are computed similarly. Then, we have the $O(1)$ recurrence

$$U_{i,j}=\begin{cases}
G_{i,j}&i=0\\
0&G_{i,j}=0\\
U_{i-1,j}+1&G_{i,j}=1
\end{cases}$$

which can be evaluated over all $U_{i,j}$ in $O(N^2)$.

## $O(N^2\ln N)$

Recall the innermost loop:

```
for k in [1, N]:
	# Range check.
	if i + k - 1 >= N or j + k - 1 >= N:
		continue
	# Top & left edges must be fully black.
	if R[i][j] >= k and D[i][j] >= k
		# Bottom & right edges must be fully black.
		and U[i + k][j + k] >= k
		and L[i + k][j + k] >= k:
		Z = max(Z, k)
```

The purpose of this loop is to find the size of the largest valid square with top-left corner at $i,j$. We know before even entering the loop that there is some $K_{i,j}=\min(R_{i,j},D_{i,j})$ that upper-bounds the size. It remains to iterate through all $k\leq K_{i,j}$ and check the bottom-right corner for $\min(U_{i,j},L_{i,j})$.

For all given $i,j$ on a upper-left-to-lower-right diagonal, all candidate bottom-right corners lie on the same diagonal. Thus, it is likely we are performing unnecessary work in (re-)checking these bottom-right corners $i+k,j+k$. To resolve this, we batch process the entire diagonal together.

Observe that the larger $k$ is, the larger $\min(U_{i+k,j+k},L_{i+k,j+k})$ will need to be. In fact, it always needs to be at least $k$ to be considered a valid candidate for bottom-right corner for the upper-left corner $i,j$. Instead of testing each $\min(U_{i+k,j+k},L_{i+k,j+k})$ against a different $k$, we can instead discount each by $k$, and test against $0$.

Let $X_{i,j}$ be this discounted minimum, pre-computed in $O(N^2)$:

$$X_{i,j}=\min(U_{i,j},L_{i,j})-i.$$

The discount $-i$ should be different for every $i,j$ upper-left corner considered on the same diagonal: if an upper-left corner has larger $i$, its candidate bottom-right corners should be discounted less. Now, we have the nice property that if $X_{i+k,j+k}\geq -i$, then $i+k,j+k$ is a valid bottom-right corner for the upper-left corner $i,j$.

We can double-check our logic by considering black cells $i,j$, for which we have $X_{i+0,j+0}\geq 1-i\geq -i$: thus, all black cells form a valid square of size $1$ by themselves.

Given a set of the $X_{i+k,j+k}$ on a diagonal, for each upper-left corner $i,j$ on the diagonal, we simply take the max $k$, $k\leq K_{i,j}$ in the set where $X_{i+k,j+k}\geq -i$. This $k$ denotes the largest valid square with $i,j$ as its upper-left corner. We can optimize this search a little by first sorting incrementally the list of $X_{i+k,j+k}$ on each diagonal. Let this sorted list be $S_d$ for the diagonal $d$ containing $i,j$, and let $Y_d$ be the list of $k$s which produced $S_d$. Then, the set of $X_{i+k,j+k}\geq -i$ can simply be found as a postfix after binary searching on $-i$. Sorting each set on each diagonal is $O(N^2\ln N)$, so it remains to consider the $k\leq K_{i,j}$ bound.

For each diagonal, we can maintain a segment tree $T_d$ aligned with $S_d$ and $Y_d$. Initially, prior to processing each diagonal, the tree’s values are set to $-N$. As we process upper-left corners $i,j$, we will continuously update the tree’s indices to reflect $Y_d$. Each query we perform on the tree will be querying the max over a postfix.

To account for the $k_0$ bound, we ensure that at the time of processing $i,j$, $T_d$ contains only the elements in $S_d$ which result from a $k\leq K_{i,j}$. Naturally, then, along each diagonal, we must process the $i,j$ in increasing sorted order of $K_{i,j}$. Prior to processing each $i,j$, we update $T_d$ such that it reflects $Y_d$ everywhere $Y_d$ is at most $K_{i,j}$. Then, binary searching on $S_d$ for index $e_0$ such that all $S_{d,e}\geq -i$ for $e\geq e_0$ gives us the postfix, and querying $T_d$ on this postfix gives the maximum element in $Y_d$ on this postfix which is also no greater than $K_{i,j}$, as desired.

Finally, recall that segment tree range queries and point updates are both $O(\ln N)$. Across a diagonal, $O(N)$ queries and updates are made to $T_d$. Over $O(N)$ diagonals, then, the cost of the segment trees is $O(N^2\ln N)$ as desired.

## $O(N^2\alpha(N))$ (for inverse-Ackermann function $\alpha$)

[work-in-progress]

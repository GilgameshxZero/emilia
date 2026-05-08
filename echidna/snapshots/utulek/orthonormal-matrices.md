<!-- emilia-snapshot-properties
Orthonormal matrices
2023/05/16

emilia-snapshot-properties -->

# Orthonormal matrices

May 16, 2023

A matrix $A$

$$A=\begin{bmatrix}
\vdots & \vdots & & \vdots\\
\mathbf{a_1} & \mathbf{a_2} & \cdots & \mathbf{a_n}\\
\vdots & \vdots & & \vdots
\end{bmatrix}$$

is orthonormal iff

1. Columns $\mathbf{a_i}$ have unit norm: $\mathbf{a_i}^T\mathbf{a_i}=1$.
2. Columns $\mathbf{a_i}$ are orthogonal with each other: $\mathbf{a_i}^T\mathbf{a_j}=0$ for $i\neq j$.
3. Thus, it must be that $A^TA=I$, and similarly $AA^T=I$. Recall that multiplication from the left acts on the rows, and multiplication from the right acts on the columns.
4. Hence, $A^T=A^{-1}$. Intuitively, $A^T$ solves

	 $$A\mathbf{x}=\mathbf{b}\iff \mathbf{x}=A^T\mathbf{b}$$

	 by taking $\mathbf{x}$ as the dot products $\mathbf{a_i}^T\mathbf{b}$. This is simply the projection of $\mathbf{b}$ onto orthonormal basis $A$, which necessarily solves $A\mathbf{x}=\mathbf{b}$.

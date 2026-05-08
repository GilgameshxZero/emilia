## $A=LDU$ (Elimination)

Gaussian elimination gives this decomposition with $L$ lower triangular, $D$ diagonal, and $U$ upper triangular, as long as $A$ has full rank.

## $A=QR$ (Orthonormal)

Acquired by the Gram-Schmidt process, which essentially gives orthonormal $Q$ by repeatedly taking out non-orthogonal components of each column, and tracking in upper-triangular $R$. The normalization of each column may be tracked by the diagonal values in $R$.

Let

$$A=\begin{bmatrix}
A_1 & A_2 & \cdots & A_n
\end{bmatrix}=QR=\begin{bmatrix}
Q_1\\
Q_2\\
\vdots\\
Q_n\\
\end{bmatrix}R$$

with the definition that $Q$ is the orthonormal decomposition of $A$. We have then, by definition,

$$Q_1=A_1/||A_1||.$$

Secondly, we compute the projection of $A_2$ onto the subspace defined by $Q_1$:

$$P_2=Q_1(Q_1^TQ_1)Q_1^TA_2.$$

Subtracting this projection from the original vector $A_2$ gives residual $R_2=A_2-P_2$. Note that I use residual to always mean the orthogonal vector to the projection, which when added to the projection, gives the residual. This is different from error, which may not be orthogonal.

Now, the residual is normalized, to give $Q_2$:

$$Q_2=R_2/||R_2||.$$

In the future, we will project to the subspace defined by column vectors $Q_1,\ldots,Q_{i-1}$. This is also the subspace defined by column vectors $A_1,\ldots,A_{i-1}$.

> When does Gram-Schmidt break down?

## $A=X\Lambda X^{-1}$ (Eigenvalue)

$X$ has columns the eigenvectors of $A$ and $\Lambda$ is diagonal with the corresponding eigenvalues, in order.

Recall that an eigenvalue $\lambda$ is paired with eigenvector $\mathbf{x}$ on a square matrix $A$ such that

$$A\mathbf{x}=\lambda\mathbf{x}.$$

Let, then, the $n$ possibly non-unique eigenvectors $\mathbf{x}_i$ be collected as columns into $X$, and place the $n$ corresponding eigenvalues into the diagonal of matrix $\Lambda$. Then we must have

$$AX=X\Lambda\implies A=X\Lambda X^{-1}.$$

Because of the nice properties of eigenvector powers, we can also write more generally

$$A^k=X\Lambda^k X^{-1}.$$

Fully unique eigenvalues guarantees $A$ may be eigendecomposed.

### Symmetry

A symmetric (Hermitian?) $A$ has all real eigenvalues and orthonormal eigenvectors, guaranteeing, eigendecomposition.

## $A=U\Sigma V^T$ (SVD)

Notably, $U,T$ are orthonormal matrices generated from $A^TA$ and $AA^T$ respectively, which renders $V^T=V^{-1}$. Intuitively, the first few terms of the SVD are those with the highest influence on the rank of $A$—and thus a truncated SVD is known as PCA.

## $A=L^TL$ (Cholesky)

Notably, the Cholesky decomposition of a square symmetric correlation matrix $A$, scaled by a random vector, will give random variables whch generate the correlation matrix.

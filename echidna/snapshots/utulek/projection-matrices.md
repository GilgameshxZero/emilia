<!-- emilia-snapshot-properties
Projection matrices
2023/05/29

emilia-snapshot-properties -->

# Projection matrices

May 29, 2023

For projection $\mathbf{p}$ of $\mathbf{b}$ onto line $\mathbf{a}$ we have with *scaling factor* $\hat{x}$

$$\mathbf{p}=\mathbf{a}\hat{x}=P\mathbf{b}.$$

Recall that dot product $\mathbf{a}^T\mathbf{b}$ is the norm of the projection of $\mathbf{b}$ onto $\mathbf{a}$, scaled by the norm of $\mathbf{a}$. This is because $\mathbf{a}^T\mathbf{b}=\mathbf{a}\cdot\mathbf{b}$ for column vectors $\mathbf{a}, \mathbf{b}$, and the dot product is the norm of both components multiplied by the cosine of the angle between them. Hence, the real norm of the projection can be found by scaling back:

$$\hat{x}=\mathbf{a}^T\mathbf{b}/\mathbf{a}^T\mathbf{a}$$

giving projection matrix $P$ onto a line as

$$P=\mathbf{a}(\mathbf{a}^T\mathbf{a})^{-1}\mathbf{a}^T.$$

Projections onto orthonormal column space $A$ follow a similar form due to similar reasoning:

$$P=\mathbf{A}(\mathbf{A}^T\mathbf{A})^{-1}\mathbf{A}^T.$$

$A$ needs not be orthonormal.

The order here obviously matters now. We have intuitively that $P^2=P$, but also that $P^T=P$ (symmetry), from the following reasoning: for vectors $\mathbf{v},\mathbf{w}$, we have their dot product in space $A$ as

$$(P\mathbf{v})\cdot\mathbf{w}=\mathbf{v}\cdot(P\mathbf{w})\implies P\mathbf{v}=\mathbf{v}P.$$

Recalling that multiplying $P$ from the right gives linear combinations of its columns, and multiplying from the left gives linear combinations of its rows. Hence, the rows and columns of $P$ must be equal and in the same order, and $P$ must be symmetric.

Then, the only order of the terms above which also preserve symmetry is the $P$ given.

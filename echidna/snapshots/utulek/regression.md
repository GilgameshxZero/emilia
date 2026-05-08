<!-- emilia-snapshot-properties
Regression
2023/05/30

emilia-snapshot-properties -->

# Regression

May 30, 2023

We use the term *regression* as opposed to *classification* when predicting quantitative outputs.

Linear regression is particularly special as it results as the same solution via four methods of estimating the same problem: to best fit a linear line to some set of data. Notably, because the least squares and maximum likelihood methods both give the same regression coefficients, this means that the (multi-variate) regression “distribution” has both mean and mode at the given coefficients.

## Derivations

Take for all examples datapoints $(x_1,y_1),\ldots,(x_n,y_n)$ with

$$X=\begin{bmatrix}
1&x_1\\
\vdots&\vdots\\
1&x_n
\end{bmatrix}$$

and $Y=[y_1 \ldots y_n]^T$.

### Projection

It is by definition that regression establishes the projection of $Y$ onto the column space of $X$. As we are manipulating the columns of $X$, the regression coefficients $B$ must be right-multiplied:

$$\hat Y=XB.$$

Of course, $B$ is a $2$-by-$1$ matrix. More generally, [projections](projection-matrices) minimize the norm of the error term $E$, a $n$-by-$1$ matrix, necessarily orthogonal to $X$’s column space:

$$Y=XB+E.$$

The norm of the error term, $|E|$, is precisely the *residual sum-of-squares* (RSS):

$$RSS(Y,\hat Y)=|E|=\sum_{i=1}^n (\hat y_i - y_i)^2=(XB-Y)^T(XB-Y).$$

Briefly, we review the projection solution. Recall for $A$ whose column space draws a line, onto which we wish to project $\mathbf{b}$:

$$\mathbf{p}=A\hat x=P\mathbf{b}.$$

Changing variables into the regression problem, we have

$$\hat Y=XB=PY.$$

For the line projection, we have by the definition of dot products

$$\hat x=\frac{A^T\mathbf{b}}{A^TA}=(A^TA)^{-1}A^T\mathbf{b}$$

and thus in the regression problem we have

$$B=(X^TX)^{-1}X^TY$$

as desired.

### Least squares

The RSS function established earlier also lends itself easily to minimization via matrix differentiation:

$$\frac{d|E|}{dB}=X^T(Y-XB)=0\implies B=(X^TX)^{-1}X^TY$$

as desired. Without matrix differentiation, we may also work directly on the summation

$$\begin{aligned}
|E|&=\sum_{i=1}^n(\hat y_i-y_i)^2=\sum_{i=1}^n(b_0+x_ib_1-y_i)^2\\
\frac{\delta|E|}{\delta b_0}&=\sum_{i=1}^n2(b_0+x_ib_1-y_i)=0\\
&\implies b_0=((\sum_{i=1}^ny_i)-b_1(\sum_{i=1}^n x_i))/n\\
\frac{\delta|E|}{\delta b_1}&=\sum_{i=1}^n2x_i(b_0+x_ib_1-y_i)=0\\
&\implies b_1=\frac{(\sum_{i=1}^n x_iy_i)-b_0(\sum_{i=1}^n x_i)}{\sum_{i=1}^n x_i^2}
\end{aligned}$$

This is a little cumbersome to solve, but the end result is surely the same as the projection solution.

### Maximum likelihood

A probabilistic view renders $X,Y$ as RVs.

### Algebraic

$$X=\begin{bmatrix}
1&x_1&x_1^2&\cdots\\
\vdots&\vdots&\vdots&\cdots\\
1&x_n&x_n^2&\cdots
\end{bmatrix}$$

## Extensions

### Higher dimensions

A trivial dimension extension to regression places higher-order terms in $X$:

$$X=\begin{bmatrix}
1&x_1&x_1^2&\cdots\\
\vdots&\vdots&\vdots&\cdots\\
1&x_n&x_n^2&\cdots
\end{bmatrix}$$

### $L_1$ distance

Instead of using squared error, the sum of absolute errors itself

$$L_1=\sum_{i=1}^n |e_i|$$

shall give the *median* instead of the *mean* of the joint distribution $Y|X$. This presents a discontinuous derivative and some difficulties in analysis.

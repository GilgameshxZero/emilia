<!-- emilia-snapshot-properties
Order statistics
2023/05/17

emilia-snapshot-properties -->

# Order statistics

May 17, 2023

It is known that the CDF of the $i$-th order statistic is $F_i(x)=P(X_{(i)}\leq x)$:

$$F_i(x)=\sum_{k=i}^n{n\choose k}(F(x))^k(1-F(x))^{n-k}$$

with $F(x)=P(X_i\leq x)$, the CDF of the population distribution.

For the $i$-th order statistic to be at most $x$, at least $i$ of the samples must be at most $x$. Each of these are disjoint events, since they have a different count of samples which are at most $x$ and at least $x$. Hence, we sum these disjoint events up to get the final CDF.

From the CDF we can derive the PDF:

$$\begin{aligned}
f_i(x)&=\frac{dF_i}{dx}(x)\\
&=\sum_{k=i}^n{n\choose k}(k(F(x))^{k-1}f(x)(1-F(x))^{n-k}\\
&~~~~~-(F(x))^k(n-k)(1-F(x))^{n-k-1}f(x))\\
&=nf(x)(\sum_{k=i}^n{n-1\choose k-1}(F(x))^{k-1}(1-F(x))^{n-k}\\
&~~~~~-\sum_{k=i}^n{n-1\choose k}(F(x))^k(1-F(x))^{n-k})\\
&=nf(x){n-1\choose i-1}(F(x))^{i-1}(1-F(x))^{n-i}
\end{aligned}.$$

The final step is due to the second summation cancelling out of the first one, once the initial term of the first summation is removed.

This PDF actually has a highly intuitive interpretation, which is as follows. First, we choose $i-1$ positions to be smaller than $x$. Of the remaining $n-i+1$ positions, we choose $1$ to be equal to $x$. This is ${n\choose i-1}(n-i+1)=n{n-1\choose i-1}$ choices for positioning.

It remains to assign numbers to each of the positions. There are $i-1$ numbers less than $x$ to assign, one number equal to $x$, and $n-i$ numbers greater than $x$ to assign. This gives the three $f$ and $F$ terms in the PDF.

> **Note**: I have come to realize that the above intuition for $f_i(x)$ is imprecise at the boundaries, where multiple numbers may be equal to $x$. It is certainly accurate as it is the correct derivative of $F_i(x)$, but that the intution above comes to the correct conclusion may be a coincidence.

## Order statistics under uniform population

Readers may recognize the form of the PDF also as close to the familiar Beta distribution:

$$\begin{aligned}
f_i(x)&=\frac{n!}{(i-1)!(n-i)!}f(x)(F(x))^{i-1}(1-F(x))^{n-i}\\
&\propto Beta(i,n-i+1)
\end{aligned}$$

Each order statistic, when interpreted as a RV itself, follows a Beta distribution, as long as $f(x)$ is uniform (i.e. population $X$ follows a uniform distribution).

Furthermore, there is a clever simplification of the distribution of order statistics differences. That is, for $j>i$, the distribution of $X_{(j)}-X_{(i)}$ is necessarily the same as the distribution of $X_{(j-i)}$ should the underlying distribution $f(x)$ be uniform.

Without directly convoluting the PDFs, we may consider instead $n+1$ samples drawn uniformly upon a circle. By splitting the circle at the first sample, we achieve the same distribution of points as we would if we had drawn $n$ points uniformly from a line. Hence, the differences of order statistics are *shift-invariant*.

More generally, we have

$$Beta(j,n-j+1)-Beta(i,n-i+1)=Beta(j-i,n-(i-j)+1).$$

Of course, the *shift-invariance* of differences also applies when adding compatible Beta distributions as well.

## Discrete RVs

Importantly, this PDF is for continuous variables, and the PDF (PMF, more accurately, but we will continue to refer to it as the PDF) for discrete RVs is slightly different. To compute it, we again use the same reasoning to compute the CDF, which should be identical, before subtracting two neighboring CDFs to compute the PDF of a single point $x$.

$$\begin{aligned}
P(X_{(i)}\leq x)&=\sum_{k=i}^n{n\choose k}(P(X\leq x))^k(1-P(X\leq x))^{n-k}\\
P(X_{(i)}< x)&=\sum_{k=i}^n{n\choose k}(P(X<x))^k(1-P(X<x))^{n-k}))\\
\implies P(X_{(i)}=x)&=P(X_{(i)}\leq x)-P(X_{(i)}< x)
\end{aligned}$$

which as you can imagine can be somewhat tedious to compute. Instead, look for applications of symmetry, or complementary computations.

Of particular note to problem-solving for order statistics is the equality for non-negative RVs

$$\begin{aligned}
E[X]&=\int_0^\infty P(X\geq x)dx
\end{aligned}$$

The intuitive explanation is that the PDF for larger $x$ are added to the answer more than the PDF for smaller $x$. In fact, the PDF for some $x$ is added exactly $x$ times in the expectation, as desired. The discrete version of this is

$$E[X]=\sum_1^\infty P(X\geq x)dx$$

with the bottom limit changing to $1$ as intuition would have it.

---

References:

1. <https://math.stackexchange.com/questions/1179371/how-to-find-distribution-of-order-statistic>.
2. <https://stats.stackexchange.com/questions/597795/roll-4-dice-whats-the-expected-value-of-the-sum-of-the-highest-3/597796#597796>.
3. <https://statproofbook.github.io/P/mean-nnrvar.html>.



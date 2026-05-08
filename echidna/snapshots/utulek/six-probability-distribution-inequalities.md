<!-- emilia-snapshot-properties
Six probability distribution inequalities
2023/05/30
utulek
emilia-snapshot-properties -->

# Six probability distribution inequalities

May 30, 2023

It occurs to me that the probability bounds most commonly used in both randomized algorithms as well as in quantitative analysis are few in number. I briefly summarize their intended usage here.

## Union bound (and subsequent Bonferroni inequalities)

Take the principle of inclusion-exclusion for the set of events $X_i$. We have

$$\begin{aligned}P(X_1\cup\ldots\cup X_N)&=P(X_1)+\ldots+P(X_N)-\ldots\\
&\leq\sum P(X_i).\end{aligned}$$

## Markov bound

For a non-negatively valued distribution, take any integer $x$. Should we collapse all the mass above $x$ into $x$, and disregard the mass below $x$ into $0$, we would have expected value $xP(X\geq x)$. Necessarily, this is less than the original expected value $E[X]$ due to the shifting and removal of masses. Hence,

$$P(X\geq x)\leq E[X]/x.$$

## Chebyshev bound

And thus from the Markov bound we have upper-bounds on the tails of any distribution, simply by taking its square to make it non-negative. Notably, we will want to recenter the distribution first for ease-of-computation.

$$\begin{aligned}
P((X-\mu)^2\geq x)&=P(X\geq \mu+\sqrt{x})+P(X\leq \mu-\sqrt{x})\\
&\leq E[(X-\mu)^2]/x=Var(X)/x.
\end{aligned}$$

## Jensen’s inequality

For some function $f$ whose second-derivative is monotone—i.e. it is either convex or concave—we have for some two points on the function, any point in between must always lie below or above the curve, depending on if $f$ is convex or concave.

For concave function $f(x)=\sqrt{x}$, we have that every point on a line between two points on $f$ must lie below $f$. Thus,

$$E[f(X)]\leq f(E[X])\iff \sqrt{E[X]}\geq E[\sqrt{X}].$$

This is expectation over any distribution—and thus covers any point on the line between the two points.

Notably, the sample standard deviation is seen on the LHS—and necessarily has non-negative bias.

## Chernoff bounds

Similar to the Chebyshev, the Chernoff bounds provide a series of exponential bounds for distribution tails. These are derived from the Markov bound on the moment generating functions, noting that they are monotone with respect to the exponent $n$.

Let us then recall the moment generating function $M_X(s)$ of a distribution $X$:

$$M_X(s)=E[e^{sX}].$$

We note that $e^{sX}$ is monotonic in $X$ and so we have the symmetric bounds

$$P(X\geq x)=P(e^{sX}\geq e^{sx}),s>0\\~\\
P(X\leq x)=P(e^{sX}\geq e^{sx}),s<0.$$

Then, as mentioned, we apply the Markov bounds

$$P(e^{sX}\geq e^{sx})\leq M_X(s)/e^{sx},s>0\\~\\
P(e^{sX}\leq e^{sx})\leq M_X(s)/e^{sx},s<0.$$

And now we must become creative with choosing any $s$ for these inequalities.

## Cauchy-Schwarz<sup>[1]</sup>

The Cauchy-Schwarz inequality states that for vectors $\mathbf{v},\mathbf{w}$, inner product $\langle\mathbf{v},\mathbf{w}\rangle$ and norms denoted by the double-bar we have

$$\lvert\langle\mathbf{v},\mathbf{w}\rangle\rvert\leq\lVert\mathbf{v}\rVert\lVert\mathbf{w}\rVert.$$

The bars on the LHS are a little unclear to me at the moment. However, taking the expectation of both sides and moving the root leads to an inequality useful in probabilities:

$$E[XY]^2\leq E[X^2]E[Y^2]$$

Amplification is a trick similar to square-root decomposition, to balance out the sides of an imbalanced equality (or inequality in this case). We begin with the trivial:

$$\begin{aligned}
\lVert\mathbf{v}-\mathbf{w}\rVert^2&\geq 0\\
\langle\mathbf{v},\mathbf{w}\rangle&\leq(\lVert\mathbf{v}\rVert^2+\lVert\mathbf{w}\rVert^2)/2
\end{aligned}$$

where the LHS is close to the LHS of Cauchy-Schwarz, but the RHS still has some ways to go. Instead, we take some $\lambda$ and modify $\mathbf{v}:=\mathbf{v}\lambda,\mathbf{w}:=\mathbf{w}/\lambda$:

$$\langle\mathbf{v},\mathbf{w}\rangle\leq\frac{\lambda^2}{2}\lVert\mathbf{v}\rVert^2+\frac{1}{2\lambda^2}\lVert\mathbf{w}\rVert^2.$$

It remains to minimize the RHS to strengthen the inequality, which we do by taking its derivative w.r.t. $\lambda$:

$$\begin{aligned}
RHS'&=\lambda\lVert\mathbf{v}\rVert^2-\lambda^{-3}\lVert\mathbf{w}\rVert^2=0\\
\lambda&=\sqrt{\lVert\mathbf{w}\rVert/\lVert\mathbf{v}\rVert}\\
\langle\mathbf{v},\mathbf{w}\rangle&\leq\lVert\mathbf{v}\rVert\lVert\mathbf{w}\rVert.
\end{aligned}$$

There is further depth to explore here (obviously), but that is beyond the scope of this note.

## References

1. Amplification: <https://terrytao.wordpress.com/2007/09/05/amplification-arbitrage-and-the-tensor-power-trick/>.

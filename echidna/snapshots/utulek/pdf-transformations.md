<!-- emilia-snapshot-properties
PDF transformations
2023/05/26

emilia-snapshot-properties -->

# PDF transformations

May 26, 2023

Consider the behavior of some function $f$ under change of variable $y$, which we call $g$. We have

$$\begin{aligned}
g(y(x))&=f(x)\text{ by definition}\\
\implies g(x)&=f(y^{-1}(x))\\
\implies g'(x)&=f'(z(x))z'(x)\text{, with }z=y^{-1}\\
\end{aligned}$$

Importantly, recall that the PDF is defined as the rate of change of the CDF. When PDF $f'$ undergoes change of variable $y'$, its altered PDF $g'$ is correspondingly multiplied by the *Jacobian factor*, which has its parallels for random vectors $\mathbf{x}$.

For example, suppose for RV $X$ we have its PDF $f'$ and a change of variable $y(X)=X^2\implies y^{-1}(Y)=z(Y)=\sqrt{Y}\implies z'(Y)=1/(2\sqrt{Y})$. The new CDF is then $g(x)=f(y^{-1}(x))=f(z(x))$ and thus the PDF is

$$g'(x)=(f'(\sqrt{x})+f'(-\sqrt{x}))/(2\sqrt{x})$$

as desired.

The mode, or maximum likelihood estimator, is similarly transformed, and due to the properties of the second derivative, may not remain trivial. However, should the transformation be linear, the new maximum likelihood estimator $\hat y$ is simply $y(\hat x)$, the trivial one.

## Example: Circle sampling

Take as example the classical problem of sampling from a circle on a plan, given only uniform generators. Obvious discarding solution aside, we may also sample on a $r,\theta$ scheme. We have

$$\theta\sim U(0,2\pi)$$

and

$$P(R=r)\propto r=cr.$$

Integrating the PDF we find $c=8$ so

$$P(R=r)=8r,P(R\leq r)=4r^2.$$

We then have some desired transformation $y$ s.t. $Z=y(R),R=y^{-1}(Z)$ and $Z$ is the uniform variable we sample:

$$P(Z\leq z)=z=P(y(R)\leq z)=P(R\leq y^{-1}(z))=4(y^{-1}(z))^2.$$

We then have $y^{-1}(z)=\sqrt{z}/2$ and hence $R=\sqrt{Z}/2$. This is confirmed in [circle-sample](../experiments/circle-sample.ipynb).

---

References:

1. <https://stats.stackexchange.com/questions/236840/different-probability-density-transformations-due-to-jacobian-factor>.
2. <https://stats.stackexchange.com/questions/14483/intuitive-explanation-for-density-of-transformed-variable>.

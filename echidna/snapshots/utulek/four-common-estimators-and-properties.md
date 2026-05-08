<!-- emilia-snapshot-properties
Four common estimators and properties
2023/05/29
utulek
emilia-snapshot-properties -->

# Four common estimators and properties

May 30, 2023

## MSE

We have for estimator $\hat\Theta$ its mean squared error (MSE):

$$
\begin{aligned}
MSE(\hat\Theta)&=E[(\hat\Theta-\Theta)^2]\\
&=E[\hat\Theta^2]-2\Theta E[\hat\Theta]+\Theta^2\\
&=E[\hat\Theta^2]-E[\hat\Theta]^2+E[\hat\Theta]^2-2\Theta E[\hat\Theta]+\Theta^2\\
&=Var(\hat\Theta)+B(\hat\Theta)^2
\end{aligned}
$$

with bias $B(\hat\Theta)=E[\hat\Theta]-\Theta$. This is, of course, slightly counterintuitive—the variance of the estimator does not cancel out the bias, if they happen to be in the same direction.

As we will see later, having the MSE vanish as the number of samples is increased means that the estimator is necessarily consistent.

## Bias and consistency

Roughly, estimators are described by two sets of orthogonal adjectives:

1. Unbiased: the bias of the estimator, $B(\hat\Theta)$, equals $0$.
2. Consistent: for any $\epsilon>0$, we have
   $$\lim_{n\to\infty}P(|\hat\Theta_n-\Theta|\geq\epsilon)=0.$$
   That is, with more samples, the estimators get arbitrarily close to the true value. This limit is the definition for convergence in probability (convergence to a constant) (as opposed to the weaker convergence in distribution, which converges to another RV).

Usually, we may determine bias simply by evaluating $E[\hat\Theta]$ in terms of the parameter we care about. Alternatively, you will see later that properties about the MMSE may make it easier to reason about bias for other estimators as well.

Thus we have four examples of different types of estimators. For the population we choose distribution $U[0, 1]$. The following examples estimate the mean of the population based on a set of samples $X_i$ drawn from it.

**Biased and inconsistent**: The mean of the population is surely $\Theta=1/2$. We simply choose estimator $\hat\Theta=0$, which is a point distribution at point $x=0$.

**Unbiased and consistent** (a good estimator): We simply choose the sample mean $\bar X=(X_1+\cdots)/n$.

**Unbiased and inconsistent**: An unbiased estimator is easy to construct: simply a distribution spread 50% at $0$, and 50% at $1$ will do. For any $\epsilon<0.5$, the probability that anything estimated will be greater than that epsilon is exactly $1$, so it is inconsistent.

**Biased and consistent**: This is perhaps the most troubling of orthogonal combinations of adjectives. Let us consider an estimator for $\Theta$, the upper bound of a population with continuous distribution $U[0,\Theta]$. We take

$$\hat\Theta=\max(X_1,\ldots).$$

Notice this is simply distribution for $n$-th order statistic of $n$ samples. We recall from order statistics that

$$P(\hat\Theta=x)\propto {n\choose n-1}(1)f(x)F(x)^{n-1}(1-F(x))^0\propto Beta(n, 1)(x/\Theta).$$

In fact, this distribution is identical to the Beta distribution, scaled up $\Theta$ times to cover the entire support of the order statistic.

Then, the mean of the estimator distribution is proportional to the mean for the Beta distribution, at $n/(n+1)\cdot\Theta$. Of course, this is biased—intuitively, the estimator always approaches the true mean from the left. Alternatively, the true parameter $\Theta$ must obviously be greater than the $n$-th order statistic, so estimating exactly the that order statistic is almost certainly an underestimate, should there be any probability mass anywehre else.

This approach comes arbitrarily close to the true parameter, however, and for that we need two facts:

1. The mean, $E[\hat\Theta]=\frac{n}{n+1}\Theta$ gets arbitrarily close to $\Theta$.
2. The variance, same as the Beta distribution itself, potentially scaled with a constant, is $Var(\hat\Theta)\propto Var(Beta(n,1))=n/(n+1)^2/(n+2)$. Obviously, this goes to $0$. The variance may also be derived directly via the formula.

Hence, it is true that $\hat\Theta$ converges in probability to $\Theta$, and indeed, this estimator is consistent.

---

Intuitively, unbiased-ness is quite strong—it requires that for any $n$ number of samples, the estimator be unbiased, while consistency only requires convergence as $n\to\infty$.

Furthermore, notably, the convergence of the MSE of the estimator to $0$ as $n\to\infty$ also proves consistency. This is because the bias and the variance both must go to $0$ (speed irrelevant). This is nearly a tautology.<sup>[2]</sup>

## Methods of estimation<sup>[3]</sup>

In my studies I have seen no more than four types of estimators commonly employed. Consider observed data $X$ and estimator $\hat\Theta$ for the following discussions.

### Maximum likelihood (ML)

We have for some observations $X$ the likelihood of generating such $X$ based on some variable $\Theta$. Then, the following function (over variousvalues of $\Theta$)

$$P(X|\Theta)$$

must have a maximum at $\Theta=\hat\Theta$ which maximizes the likelihood of seeing $X$ under that schema. This is the maximum likelihood estimator.

To compute the ML estimator, usually one takes the likelihood function $L(\Theta)$ and maximizes it via derivatives. To make this easier, sometimes the log-likelihood is used, as $\log$ is monotonic.

### Method of moments (MoM)<sup>[7]</sup>

The MoM estimators are straightforward. We have, of course, the $s$-th moments of the population $X$:

$$E[X^s]$$

but we also have the moments of the random sample:

$$E[x^s].$$

We may equate the two moments with each other, for some values of $s$, and solve for the parameter in question. Notably, we begin with $s=1$, and use increasingly larger values of $s$, as we need to estimate more parameters.

Alternatively, we may utilize the *moment around the mean* for $s\geq 2$:

$$E[(X-E[X])^s]$$

for different estimators. The properties of these estimators need to be analyzed like any other.

### Maximum a posteriori (MAP)

The MAP estimator is very related to the ML estimator. Notably, for data $X$, it is the value $\hat\Theta$ which maximizes

$$P(\Theta|X)=P(X|\Theta)P(\Theta)/P(X)$$

which is the same value of $\hat\Theta$ which maximizes

$$P(\Theta|X)\propto P(X|\Theta)P(\Theta).$$

Should the a priori distribution $P(\Theta)$ be uniform, then the ML estimator, which maximizes $P(X|\Theta)$ is necessarily the same as the MAP estimator. Thus—MAP can be seen as a generalization on ML.

For example, in estimating the bias $p$ of a coin given $14$ flips of which $10$ resulted in heads, the ML estimator is obviously $\hat p_{ML}=10/14$, and because we have no prior distribution, the reasonable conjugate prior to choose here is the Beta distribution, which must mean that the prior is uniform. As such $\hat p_{MAP}=10/14$ as well.

### Minimum mean squared error (MMSE)<sup>[4]</sup>/Expected a posteriori (EAP)<sup>[5]</sup>

Consider again the distribution $\Theta|X$, defined over the support of $\Theta$: instead of choosing the mode of this distribution, which gives the MAP estimator, we choose the mean. This gives the MMSE estimator:

$$\hat\Theta_{MMSE}=E[\Theta|X].$$

Very conveniently, this estimator $\hat\Theta$ minimizes the mean squared error from the real statistic $\Theta$:

$$\hat\Theta=\min_\theta E[(\Theta-\theta)^2].$$

This may be verified directly via calculus and the use of the law of iterated expectations (LIE) near the end:

$$E[X]=E[E[X|Y]].$$

## Example: stimator for uniform RV

We have for some population generated from $\mathbf{X}=U[0,\Theta]$ samples $\mathbf{x}=x_1,\ldots,x_n$. The estimator $\hat\Theta=\max(x_i)=x_{(n)}$ is trivially the ML estimator. That is, the likelihood of achieving samples $\mathbf{x}$ is

$$L(\mathbf{X}|\Theta)=(1/\Theta)^n\text{ when }\Theta\geq X_{(n)}$$

which is minimized when $dL/d\Theta=0=n\ln\Theta(1/\Theta)^{n-1}\implies\hat\Theta=-\infty$, but considering the lower bound on likelihood, we must have $\hat\Theta_{ML}=\mathbf{x}_{(n)}$.

It is straightforward that this is a biased estimator, since the $n$-th order statistic of a set of $n$ numbers chosen from a uniform population has mean $n/(n+1)$. In fact, this means that $(n+1)/n\cdot \mathbf{x}_{(n)}$ should be an unbiased estimator. We now claim that this is also the MMSE estimator. 

Before moving forward, let us also make claim to its consistency. We see that  $(n+1)/n\cdot\mathbf{x}_{(n)}$ tends to Beta-distributed, with $\alpha=n$ and $\beta=1$. We have that the variance of a Beta distribution is

$$\alpha\beta/O(\alpha^3+\beta^3)$$

which tends to $0$ as $n$ tends to $\infty$. Thus, in

$$MSE(\hat\Theta_{MMSE})=Bias(\hat\Theta_{MMSE})+Var(\hat\Theta_{MMSE})$$

the latter term tends to $0$. Then, the bias is $)$ as discussed previously, so this estimator is consistent. We show consistency of $\hat\Theta_{ML}$ in a similar way. Of course, the variance is the same variance of the Beta, which goes to $0$. For the bias we now know

$$Bias(\hat\Theta_{ML})=E[-\mathbf{x}_{(n)}/n]=-n/(n+1)/n=-1/(n+1)$$

from the mean of the Beta distribution. This obviously tends to $0$ as well, so $MSE(\hat\Theta_{ML})$ tends to $0$. So, this estimator is consistent.

To show that this is indeed the MMSE estimator, I will be a bit more carful with notation, since not doing so will make things more difficult later on. We see that $\mathbf{X}$ is a random variable, but so is $\Theta$. $\mathbf{x}$ and $\theta$ are samples drawn from those RVs. Bayes’ rule gives

$$P(\Theta=\theta|\mathbf{X}=\mathbf{x})P(\mathbf{X}=\mathbf{x})=P(\mathbf{X}=\mathbf{x}|\Theta=\theta)P(\Theta=\theta).$$

But of course we have no reason to assume any particular prior $P(\Theta=\theta)$ so we let it be uniform. Furthermore, since $\mathbf{x}$ is fixed, $P(\mathbf{X}=\mathbf{x})$ is constant, so we have

$$P(\Theta=\theta|\mathbf{X}=\mathbf{x})\propto P(\mathbf{X}=\mathbf{x}|\Theta=\theta).$$

Here it is tempting to automaticallly say that $P(\mathbf{X}=\mathbf{x}|\Theta=\theta)\propto L(\mathbf{X}=\mathbf{x}|\Theta=\theta)=\theta^{-n}$. This is correct. But this will not give the correct answer—in fact, it will give as answer

$$\frac{n-1}{n-2}x_{(n)}$$

which is obviously wrong. I have spent some time thinking about why this is the case. I believe it is very nuanced. As part of computing the scaling factor involved in all the proportions, we must compute

$$\int P(\Theta=\theta|\mathbf{X}=\mathbf{x}).$$

Doing this naively from $\theta=\mathbf{x}_{(n)}$ to $\theta=\infty$ gives $P(\Theta=\theta|\mathbf{X}=\mathbf{x})=(n-1)\mathbf{x}_{(n)}^{n-1}\theta^{-n}$. But I claim that the bounds are wrong. Specifically, in declaring some $\mathbf{x}_{(n)}$ as the maximum sample, we invalidate the earlier claim that $P(\mathbf{X}=\mathbf{x}|\Theta=\theta)=L(\mathbf{X}=\mathbf{x}|\Theta=\theta)=\theta^{-n}$. I don’t quite know the resolution to this problem. Perhaps we can try conditioning each side of Bayes’ rule on $\mathbf{x}_{(n)}=m$?

The easier resolution is to take the random variable $\mathbf{x}_{(n)}$ and create an event in which it is equal to $x_{(n)}$ directly. Then we have for Bayes’ rule

$$P(\Theta=\theta|\mathbf{x}_{(n)}=x_{(n)})\propto P(\mathbf{x}_{(n)}=x_{(n)}|\Theta=\theta).$$

The RHS is evaluated via likelihood, or via order statistics. We have

$$P(\mathbf{x}_{(n)}=x_{(n)}|\Theta=\theta)=nx_{(n)}^{n-1}\theta^{-n}.$$

For some reason, I cannot evaluate $\hat\Theta_{MMSE}=E[\Theta|\mathbf{X}=\mathbf{x}]$ directly.<sup>[6]</sup> I must take the expectation of the RHS directly and then rescale based on $\theta$. I am unclear why this is. In any case, we get the answer

$$\hat\Theta_{MMSE}=\frac{n+1}{n}\mathbf{x}_{(n)}$$

as an unbiased, consistent estimator.

---

References:

1. <https://stats.stackexchange.com/questions/88800/intuitive-reasoning-behind-biased-maximum-likelihood-estimators>.
2. <https://stats.stackexchange.com/questions/280684/intuitive-understanding-of-the-difference-between-consistent-and-asymptotically>.
3. <https://math.mit.edu/~rmd/650/estimation.pdf>.
4. <https://www.probabilitycourse.com/chapter9/9_1_5_mean_squared_error_MSE.php>.
5. <https://towardsdatascience.com/mle-map-and-bayesian-inference-3407b2d6d4d9>.
6. <https://math.stackexchange.com/questions/2246222/unbiased-estimator-of-a-uniform-distribution>.
7. <https://online.stat.psu.edu/stat415/lesson/1/1.4>.

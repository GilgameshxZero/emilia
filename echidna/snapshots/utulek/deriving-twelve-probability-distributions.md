<!-- emilia-snapshot-properties
Deriving twelve probability distributions
2023/05/15
utulek
emilia-snapshot-properties -->

# Deriving twelve probability distributions

May 15, 2023

In my studies I have encountered commonly only 5 discrete and 7 continuous distributions.

## Uniform (discrete and continuous)

There is little to be said of the uniform distributions, except that they are the most basic of the distributions. In fact, the uniform distribution forms the basis of scenarios which help derive the following distributions.

## Normal

The Normal distribution is forced by two requirements.

1. Its joint distribution, $P_{X_1,X_2}(x_1,x_2)$ sees independent $X_1,X_2$.
2. $P_{X_1,X_2}$ is rotationally symmetric.

Of note is the integral

$$\int_0^\infty e^{-x^2/2}dx$$

which via transformation into polar coordinates shall equal $\sqrt{2\pi}$, thus giving the normal distribution its constant.

## Geometric, Exponential, Gamma, and Poisson

Consider this scenario:

> A bakery sees on average $\lambda<1$ customers per minute, whose arrival times are distributed uniformly.

Questions on this scenario lead to the four distributions.

**Geometric**: One way this scenario could be fulfilled is by the following underlying mechanic: with probability $\lambda$, one customer arrives in each minute. Otherwise, no customer arrives in that minute. In which minute will the next customer arrive?

We take PDF $P(x)$ as the probability that the next customer arrives in the $x$-th minute. It must be then that for $x-1$ minutes prior, no customers arrive:

$$P(x)=(1-\lambda)^{x-1}\lambda.$$

It is self-evident that the mean should be $1/\lambda$. The variance is not easy to compute, but is $(1-\lambda)/\lambda^2$, and is had by separating $E[X^2]=E[X(X-1)]+E[X]$.

**Exponential**: Dropping the discrete assumption earlier, how many minutes will pass before the next customer arrives?

The Exponential distribution is simply the continuous analogue of the geometric. Should we divide a minute into $n$ periods, we would expect $\lambda/n$ customers per period. We take $n$ to infinity.

For $x$ minutes to pass before the next customer arrives, then, we would see $xn$ periods with no customer and a final period with a customer:

$$P(x)\approx \lim_{n\to\infty}(1-\lambda/n)^{xn}(\lambda/n)\propto e^{-\lambda x},$$

recalling that $e^x=\lim_{n\to\infty}(1+x/n)^n$.

Indeed, $P(x)$ must sum to $1$, and we have

$$\int_0^\infty e^{-\lambda x}=\Big[-e^{-\lambda x}/\lambda\Big]_0^\infty=1/\lambda$$

so $P(x)$ must equal $\lambda e^{-\lambda x}$.

As the continuous analogue of the geometric, the mean should remain the same, at $1/\lambda$. The variance is computed by a double integration by parts, and is $1/\lambda^2$.

**Gamma**: How many minutes will pass before the next $n$ customers arrive?

Immediately, this is simply the sum of $n$ exponential distributions, and by its name, must invoke the Gamma function. To intuit the Gamma function, consider how it must extend the factorial function with an integral. Certainly, this integral must be evaluated by cascading integration by parts:

$$\Gamma(n+1)= \int_0^\infty x^ne^{-x} dx.$$

Perhaps the $-x$ in the exponent is unintuitive. But consider this: all the _trash_ terms in the integration by parts must vanish, leaving only the factorial term remaining. $x^n$ vanishes the $0$ limit, so the $e$ term must vanish the $\infty$ limit. Hence, the exponent must be negative.

Alternatively, consider the weak Sterling’s approximation derived earlier:

$$\Gamma(n+1)\approx f(n)=n!\approx (n/e)^n.$$

Given that $\Gamma$ must be an integral, the inside of the integral must then be similar to the derivative of $(n/e)^n$. Though weakly linked, there is indeed some similarity in form between $x^ne^{-x}dx$ and $(n/e)^n$.

To intuit the PDF $P_G(x)$ of the Gamma distribution, we take first the sum of two exponential RVs with PDF $P(x)$:

$$
\begin{aligned}
&\mathbb{P}(Gamma(n=2,\lambda)=x)=P_G(x)\\
&\propto\int_{i=0}^x P(i)P(x-i)di\\
&=\int_{i=0}^x \lambda^2 e^{-\lambda i}e^{-\lambda(x-i)} di\\
&=\lambda^2\Big[e^{-\lambda x}i\Big]_{i=0}^x\\
&=\lambda^2 e^{-\lambda x}x\\
&=\lambda^n x^{n-1}e^{-\lambda x}
\end{aligned}
$$

of which the final line may be inferred from the general form on the line prior. It remains to scale this PDF to sum to unity. To do this, of course, we invoke the $\Gamma$ function, with a clever substitution $y=\lambda x$:

$$
\begin{aligned}
P_G(x)&=\lambda^n (y/\lambda)^{n-1}e^{-y}\\
\int_{x=0}^\infty P_G(x)dx&=\int_{x=0}^\infty y^ne^{-y}(1/x)dx\\
&=\int_{y=0}^\infty y^ne^{-y}(1/x)(1/\lambda)dy\\
&=\int_{y=0}^\infty y^{n-1}e^{-y}dy\\
&=\Gamma(n).
\end{aligned}
$$

And thus we scale the proportional PDF earlier by $1/\Gamma(n)$:

$$P_G(x)=\lambda^nx^{n-1}e^{-\lambda x}/\Gamma(n).$$

In other literature, you may see the variables $n$ replaced with $\alpha$ and $\lambda$ replaced with $\beta$.

As the Gamma distribution is the sum of i.i.d. exponential distributions, its mean is necessarily $n/\lambda$ and its variance $n/\lambda^2$.

**Poisson**: Suppose now that $\lambda \geq 1$, for ease of notation. How many customers will arrive in a minute?

Again, the approach of taking number of moments $n\to\infty$ is fruitful. We have for large $n$ moments per minute that with probability $\lambda/n$ a customer is slated to arrive. So for $x$ customers to arrive in the minute, $x$ of those $n$ moments must have a customer, and the rest must not.

$$
\begin{aligned}
P(x)&= \lim_{n\to\infty}{n\choose x}(\lambda/n)^x(1-\lambda/n)^{n-x}\\
&=\lim_{n\to\infty}\frac{n!}{x!(n-x)!}(\lambda/n)^x(1-\lambda/n)^{-x}e^{-\lambda}\\
&=e^{-\lambda}\lim_{n\to\infty}\frac{n!}{x!(n-x)!}(\frac{\lambda}{n}\cdot\frac{n}{n-\lambda})^x\\
&=e^{-\lambda}\frac{\lambda^x}{x!}\lim_{n\to\infty}\frac{n\cdot(n-1)\cdots(n-x+1)}{(n-\lambda)^x}\\
&=\lambda^x e^{-\lambda}/x!.
\end{aligned}
$$

By definition, the mean is $\lambda$. The variance, perhaps surprisingly, is also $\lambda$. The intuition to this lies in the binomial distribution: that when $n$ intervals are used, the mean is $n(\lambda/n)=\lambda$, but the variance is $n(\lambda/n)(1-\lambda/n)$, but $1-\lambda/n$ vanishes to unity as $n$ goes to $\infty$, and so the variance is the same as the mean, at $\lambda$.

Notably, the Poisson distribution is also the limit of the Binomial distribution taken $n\to\infty$.

## Bernoulli, Binomial, and Beta

There is little to be said of the Bernoulli and Binomial: it is too common, and it is easily derived. The mean is $np$, and the variance $np(1-p)$, which is useful in gaining intuition for the Poisson distribution.

To derive the Beta distribution, consider the following scenario.

> A possibly unfair coin is flipped $14$ times, and it comes up heads $10$ times and tails $4$ times. Suppose if you guess the bias of the coin, you win $1$, and otherwise win nothing. What do you guess?

As we are penalized only on the wrongness of our guess, it is correct to use the maximum likelihood estimator on the coin bias. As one may expect, the most likely bias of the coin as a result of observing the $14$ flips is that it comes up heads $10/14$ times.

A (more natural) penalty invokes the Beta distribution.

> Suppose now, instead, that you will be given $1 if the next flip comes up heads and nothing otherwise. How much are you willing to pay to play this game?

And thus we are asked for the likelihood of a certain bias, given our observations, and the mean of this distribution. This is the mean of the Beta distribution $Beta(\alpha=11,\beta=5)$.

From Bayes’ rule, we have for some bias $x$

$$
\begin{aligned}
P(x|\text{data})P(\text{data})&=P(\text{data}|x)P(x)\\
P(x|data)&\propto P(x)x^{10}(1-x)^4
\end{aligned}
$$

Recall that a conjugate prior distribution is a distribution such that if it were the prior distribution before some data, then it would remain the posterior distribution after updating on the data. As the Beta distribution is the conjugate prior of the binomial (and Bernoulli, negative binomial, and geometric) distribution, we may assume that $P(x)$ is uniform when we have no prior, and thus

$$Beta(\alpha, \beta)(x)\propto x^{\alpha-1}(1-x)^{\beta-1}.$$

It remains to scale this—to compute

$$c=\int_0^1 x^{\alpha-1}(1-x)^{\beta-1}dx.$$

I have referred an excellent answer by Qiaochu Yuan, which derives

$$c=\Gamma(\alpha)\Gamma(\beta)/\Gamma(\alpha+\beta).$$

Notably, the Beta distribution has mean at $\alpha/(\alpha+\beta)$, which is different from the MLE estimator/mode we derived earlier of $10/14$. The mode, however, is as expected, at $(\alpha-1)/(\alpha+\beta-2)$. The variance for the Beta distribution may be derived directly from the formula.

## $\chi^2$ and T

These final two distributions are mostly seen in hypothesis testing.

The $\chi^2$ distribution with $n$ degrees of freedom is the sum of $n$ standard normal random variables. Should their variances differ from $1$, the distribution may be scaled as well. Notably,

$$\chi^2(n)=Gamma(n/2,1/2)$$

and in partular $N(0,1)^2=Gamma(1/2,1/2)$, because

$$
\begin{aligned}
N(0,1)^2(x)&=(N(0,1)(\sqrt{x})+N(0,1)(-\sqrt{x}))(\frac{d}{dx}(\sqrt{x}))\\
&=2N(0,1)(\sqrt{x})(1/(2\sqrt{x}))\\
&=\frac{2}{\sqrt{2\pi}}e^{-x/2}/(2\sqrt{x})\\
&\propto \frac{1}{\sqrt{x}}e^{-x/2}.
\end{aligned}
$$

Notably, the extra factor of $\frac{d}{dx}(\sqrt{x})$ is necessary in the transformation of variables of continuous distributions (derived in [pdf-transformations](pdf-transformations)). One will recall the Gamma distribution PDF thusly:

$$
\begin{aligned}
Gamma(n,\lambda)(x)&=\lambda^nx^{n-1}e^{-\lambda x}/\Gamma(n)\\
Gamma(1/2,1/2)(x)&\propto \frac{1}{\sqrt{x}}e^{-x/2}
\end{aligned}
$$

as desired. We have, of course, that sums of Gamma distributions give Gamma distributions, and so we have $\chi^2(n)=Gamma(n/2,1/2)$.

The T-distribution limits to the standard normal.

---

References:

1. <https://www.behind-the-enemy-lines.com/2008/01/are-you-bayesian-or-frequentist-or.html>: A discussion about differences between Bayesian and frequentist interpretations, which leads naturally into the intuition for the Beta distribution.
2. <https://math.stackexchange.com/questions/3528/beta-function-derivation>.

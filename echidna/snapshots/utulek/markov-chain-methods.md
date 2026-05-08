<!-- emilia-snapshot-properties
Markov chain methods
2023/05/31

emilia-snapshot-properties -->

# Markov chain methods

May 31, 2023

The general methods of solving first-time-to-state or absorption-likelihood-from-state are easy to remember.

That is, to solve for the expected time until first entering a state $s$, we compute variables $x_i$ signifiying the expected time until entering state $s$, when starting from state $i$. Then, necessarily, this reduces to a linear algebra problem with $n$ as the total number of states.

Similarily, to compute the expected time until absorption at some component or node, we may compute variables $x_i$ for the same metric, when starting at state $i$, and solve a linear system.

> Question: Consider not just the mean, but the distribution of time until absorption or first time to state, starting from some state $s$. What is this distribution?

## Unbounded Markov walk/Gambler’s ruin

In the classical Gambler’s ruin question, we ask:

> Currently we are at state $1$. At state $0$ is an absorption node we call *loss*. With probability $p$ we transition from some state $s$ to $s+1$, and with probability $1-p$ we transition to $s-1$. What is the likelihood of absorption from every state?

The crucial observation in this problem is that absorption from state $2$ requires us to travel to $1$ first, then be absorbed from $1$. That is, if the likelihood of absorption from $1$ is $x_1$, then $x_2=x_1^2$. But we also have

$$x_1=1-p+px_2\implies px_1^2-x_1+1-p=0$$

which is a solvable quadratic with solutions

$$x_1=1\pm\sqrt{1-4p+4p^2}/(2p)=1,(1-p)/p.$$

So we know that this analysis is only valid if $p\leq 0.5$. Luckily, oftentimes we are able to switch the directions of $p$ and $1-p$ to establish this, and on the border we have $p=0.5$ and $x_i=1$ for all $i$.

---

This is a fine problem, and may be modeled by the naive Markov chain approach as well, but with some difficulty in dealing with infinite series. We now extend this problem to have two absorbing states at $0$ and $n+m$. The starting state is now $n$, with the same probability $p$ of moving right.

I bring up now a brief argument why absorption must eventually occur when there are two boundaries. Consider that each step is a Bernoulli RV of $\pm1$, and thus their sum is binomial, with variance tending to $\infty$ as the number of steps also tends to $\infty$. Thus, the mass of the binomial RV that is between $0$ and $n+m$ must vanish, and hence absorption must occur with likelihood $1$ as steps tends to $\infty$.

As before, we may compute the likelihood of loss $x_1=(1-p)/p$. Notably, should $p<0.5$, we may instead define loss at $n+m$ and thus compute the likelihood of loss at $x_{n+m-1}$. We then consider the likelihood of left-absorption $l_n$ from starting state $n$ (the argument must be reversed for $p<0.5$):

$$l_n=x_n-(1-l_n)x_{n+m},$$

that is, to be left-absorbed is to lose ($x_n$) without being right-absorbed first. The likelihood of being right-absorbed first is $(1-l_n)$ (the likelihood of not being left-absorbed first, since absorption must happen) times $x_{n+m}$ (losing after becoming right-absorbed).

And thus we may solve for $l_n$, remembering that $x_i=x_1^i$.

There is little more to be said about this problem.

## MCMC

Markov Chain Monte Carlo is frankly a trivial method to estimate the stationary distribution of a Markov chain. Basically, we run simulations, and stop at some point, and let the estimator of the stationary distribution be the probability derived from these multiple simulations.

It must be asked: what properties does the MCMC estimator have? Is it unbiased or consistent? Consistency is intuitively obvious, while unbiased-ness should only arise when the starting state distribution is unbiased. But otherwise, it seems like a fairly safe procedure.

> It remains to be determined how many steps the simulation should be run.

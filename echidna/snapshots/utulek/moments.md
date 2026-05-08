<!-- emilia-snapshot-properties
Moments
2023/05/31

emilia-snapshot-properties -->

# Moments

May 31, 2023

Recall the $s$-th moment of a random variable $X$ is defined as 

$$E[X^s].$$

Notably, the first moment is the mean, and the difference between the first and second moments is the variance.

This, of course, recalls the definition of the moment generation function $M_X(s)$ of a random variable $X$:

$$M_X(s)=E[e^{sX}].$$

This has an easy interpretation of why it is called the *moment generating function*:

$$M_X(s)=1+xE[X]+s^2E[X^2]/2!+\ldots$$

and thus we see the moments in the coefficients of each of the terms in this polynomial. Notably, taking the $n$-th derivative of the moment generating function, we have

$$\frac{d^nM_X}{ds^n}(s)=E[X^n]+sE[X^{n+1}]+\ldots$$

and thus this polynomial taken at $s=0$ will give the $n$-th moment of $X$, as desired.

## References

1. <https://math.stackexchange.com/questions/1467581/what-does-it-mean-for-a-probability-distribution-to-have-a-finite-fourth-moment>.

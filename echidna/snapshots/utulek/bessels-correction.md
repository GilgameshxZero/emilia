<!-- emilia-snapshot-properties
Bessel correction
2023/06/01

emilia-snapshot-properties -->

# Bessel correction

June 1, 2023

It is well-known and intuitive that the maximum-likelihood sample estimate for population variance is

$$\hat\sigma^2=\frac{1}{n}\sum_{i}(x_i-\hat x).$$

One will recall that MLEs are derived by maximizing via derivative the likelihood—or, alternativley, the log-likelihood—of observing the given samples, given the stated value of the estimate.

Intuitively, the MLE of a distribution is its mode, while the median is the minimum-distance estimator, and the mean is the least-squares estimator.

However, it is also well-known that the MLE for population variance is also biased. Intuitively, the MLE for the mean, $\hat x$, is the number which minimizes $\hat\sigma^2$. Since it is likely that $\hat x\neq x$, the true population variance is likely to be greater than the MLE variance. Hence, we have Bessel’s correction:

$$\hat{s^2}=\frac{1}{n-1}\sum_{i}(x_i-\hat x).$$

The sample standard deviation is derived with this correction, with

$$\hat s^2=\sqrt{\hat{s^2}}.$$

But do note that the square-root function is not linear, but is concave, so Jensen’s inequality applies here and the sample standard deviation is usually an overestimate of the population standard deviation.

---

Reference:

1. <https://en.wikipedia.org/wiki/Bessel%27s_correction>.
2. <https://stats.stackexchange.com/questions/88800/intuitive-reasoning-behind-biased-maximum-likelihood-estimators>.

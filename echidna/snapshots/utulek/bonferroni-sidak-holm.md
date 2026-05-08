<!-- emilia-snapshot-properties
Bonferroni, Sidak, Holm
2023/05/29

emilia-snapshot-properties -->

# Bonferroni, Sidak, Holm

May 29, 2023

Both the Bonferroni and Šidák (accents henceforth omitted for brevity) corrections deal with the possibility of _p-hacking_—that is, running multiple trials and picking the one with the lowest p-value to support the alternative hypothesis. For $n$ similar trials run at a rejection p-value level of $\alpha$, the Sidak correction computes an adjusted level $\alpha_s<\alpha$ which must be satisfied by any of the trials to constitute significance.

Specifically, the probability that all trials are insigificant is to be bounded by $(1-\alpha_s)^n$. Then, $\alpha=1-(1-\alpha_s)^n$ gives the probability and the level that any of them are significant—hence, solving for $\alpha_s$ gives the required level for significance at level $\alpha$ over the family of trials. We thus have

$$\alpha_s=1-(1-\alpha)^{1/n}.$$

It is noted that this requires pairwise independence among the trials. Should independence be violated, the *rank* $n$ essentially decreases, and the adjusted level $\alpha_s$ goes up as well—meaning that a family of trials which rejects the null under Sidak should also reject then null under Sidak for less-than-full rank trials.

**Edit**: Dependence in the other direction—negative dependence—is concerning and will rather make the effective *rank* $n$ increase, which invalidates Sidak. In this case, we must use Bonferroni.

---

The Bonferroni correction is similar, but simpler, and does not stipulate independence, but is also less powerful as an expected result. For $n$ trials at intended level $\alpha$, the Bonferroni-corrected level is

$$\alpha_b=\alpha/n.$$

---

Both corrections lower the likelihood of committing Type I errors—false positives. The Sidak correction, being higher-powered, offers lower Type II rates than Bonferroni. To further lower the Type II rates (failing to reject when false i.e. false negative), we may apply the Holm method.

That is, we sort the trial p-values in non-decreasing order, and perform a modified Bonferroni on each—with modified levels $\alpha/n,\alpha/(n-1),\ldots,\alpha/1$. Intuitively, it must have taken on average $n-i$ trials to get the $i$-th lowest p-value, so we may compare it directly at that modified level, since all p-values before it are smaller anyway. Admittedly, it’s intuitive, though a little tricky.

Holm’s method may also be used with Sidak corrections at each step.

---

References:

1. <https://stats.stackexchange.com/questions/20825/sidak-or-bonferroni>.

<!-- emilia-snapshot-properties
Sterling’s approximation intutions
2023/05/29

emilia-snapshot-properties -->

# Sterling’s approximation intutions

May 29, 2023

We first derive a suboptimal approximation for $n!$, which bears some similarity to the derivation of the bound for a harmonic and prime harmonic series:

$$\begin{aligned}
f(n)&=n!\\
\implies \ln f(n)&=\ln 1+\ln 2+\cdots+\ln n\\
&\approx \int_1^n (\ln x)dx\\
&=\Big[x\ln x-x\Big]_1^n &\text{(by parts)}\\
&=n\ln n-n+1\\
\implies f(n)&\approx (e^{\ln n})^n/e^n/e\\
&\approx (n/e)^n.
\end{aligned}$$

Sterling’s approximation includes an additional (asymptotic) factor:

$$f(n)=n!\approx \sqrt{2\pi n}(n/e)^n$$

which is most easily derived using the Gamma function, and transforming parts of the integral into two dimensions.

Indeed, one will notice that the $\sqrt{2\pi n}$ factor bears similarity to the constant seen in the Normal distribution, which also uses a similar integral transformation technique. This similarity is likely the most intuitive way to remember the constant factor for Sterling’s.

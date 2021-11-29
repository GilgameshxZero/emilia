<div class="next-subtitled"></div>

# Support Vector Machines 

September 8, 2021

Support vector (SV) machines (SVM) build on (Vapnik-Chervonenkis) VC theory to perform regression and thus classification. Classification estimates the relationship between a set of independent variables and a categorical dependent; regression is a generalization where the dependent is not necessarily categorical (though commonly Borel-measurable).

Let $\mathcal{X}$ denote the input space (the independent variables); for flattened 32-by-32 pixel RGB images, $\mathcal{X}=([0,255]^3)^{32^2}$. Similarly, let $\mathcal{Y}$ denote the output space. For dataset $\{(x_1,y_1),\ldots,(x_l,y_l)\}\subset \mathcal{X}\times\mathcal{Y}$, then, …

SVMs are parameterized with a *kernel function* $K:\mathcal{X}\times \mathcal{X}\to\mathbb{R}$. Loosely, $K$ defines some measure of distance between pairs of datapoints in the input space (though this may not satisfy traditional distance measure assumptions, e.g. triangle inequality).

## Citations

1. Smola AJ, Schölkopf B. 2004. A tutorial on support vector regression. Statistics and Computing. 14: 199-222.

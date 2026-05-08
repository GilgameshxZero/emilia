<!-- emilia-snapshot-properties
CF1805 notes
2023/04/04

emilia-snapshot-properties -->

# CF1805 notes

April 4, 2023
Yang Yan

## [CF1805C](https://codeforces.com/contest/1805/problem/C)

Spent too long thinking about tangents on conic sections. Should have immediately tried to intersect the line and parabola, then BS.

## [CF1805D](https://codeforces.com/contest/1805/problem/D)

Tree diameter problem. “Combing” outward on diameter halves works, but more simply, taking heights from ends of diameter is easier. This appears nontrivial to prove.

## [CF1805E](https://codeforces.com/contest/1805/problem/E)

Easy problem after exploring MAD properties.

## [CF1805F1](https://codeforces.com/contest/1805/problem/F1)

Both binary-search and set/PQ approach for computing $F$ work. Key observation is that the lowest element of $F$ may be used to shift the entire array, while preserving order. This technique generalizes to order-based computations on modded answers.

<!-- emilia-snapshot-properties
Linear max gap
2022/12/01
altair
emilia-snapshot-properties -->

# Linear max gap

December 1, 2022
Yang Yan

A friend Sanjeev introduced the following problem over Thanksgiving weekend:

> Given a list of unsorted integers $A_1,\ldots,A_N$, find in $O(N)$ the maximum difference between consecutive integers of the sorted list.

To invalidate a naive bucket and radix sort, assume each $A_i$ is up to $4^N$, such that a radix sort runs in $O(N\ln N)$ and the bucket sort in $O(4^N)$. However, with some suspension of belief, assume that other basic arithmetic and comparisons are constant time.
Anyway, I detail my approach below.

## Iterative ideation

I began the problem thinking through iterative solutions, and this came to mind:

> For each prefix $A_1,\ldots,A_i$, track the max gap $G$ for integers of that prefix, alongside the $A_{min},A_{max}$. For $A_{i+1}$, if it falls before $A_{min}$ or after $A_{max}$, potentially update $G$ if either $A_{min}-A_{i+1}$ or $A_{max}-A_{i+1}$ are greater. Otherwise, the max gap $G$ cannot increase.

Of course, it’s easy to spot the error here: should $A_{min}\leq A_{i+1}\leq A_{max}$, $G$ cannot increase—but it can decrease. Unfortunately, we don’t have the information to determine whether or not this happens.

Intuitively, we need to have information about all other gaps of the prefix, to determine if $G$ decreases with $A_{i+1}$. Looking back, most iterative solutions are some form of DP, and for $O(N)$ iterations, we need to keep at most $O(1)$ information per state. Then, DP approaches are probably impossible.

## Square root decomposition

This is a fairly common CP technique, most commonly in Mo’s algorithm-like scenarios. I’m not too confident about this approach, though, because the final runtime usually involves a $\sqrt{N}$ factor.

The core intuition behind Mo’s algorithm is that each of the $\sqrt{N}$ buckets cover a range of $\sqrt{M}$ rather than $M$, and whatever updates need to happen are linear to the range. So processing a bucket ends up being $O(\sqrt{NM})$, with all bucket processing being $O(N\sqrt{M})$, which usually eats the runtime of resetting buckets $O(\sqrt{N})$ times.

The problem here is that processing a number is hardly dependent on the range of the number—it is dependent on the number of numbers already processed, in that bucket. I fail to see a solution to this.

## Back to iterative

I make the mistake of jumping back into DP at this time. We get the initial gap $G=A_{\text{second min}}-A_{min}$ for free. We can then partition the range $M=A_{max}-A_{min}$ of the numbers into buckets of size $G$. Numbers within the same bucket don’t need to be considered, and provided a reasonable distribution of $A_i$, we expect around $O(N)$ buckets. This means we can hash the numbers into buckets in $O(N)$ time. The assumption on the distribution is a red herring we may be able to solve later with randomization.

I also make the mistake of jumping back into iterative procedures. I’m not sure where I was going with the following proposal—there’s a lot wrong with it.

> Given the current max gap $G$, and a current *left pivot* $A_{lp}$, assume that the max gap is either $G$ or greater, or occurs to the right $A_{lp}$. To begin, swap $A_{min}$ to the beginning of the array and set it as left pivot, with $G$ as the initial gap $A_{\text{second min}}-A_{min}$.
> 
> For each new number $A_{i+1}$, if $A_{i+1}\geq A_{lp}+G$, hash $A_{i+1}$ to its bucket. Otherwise, $A_{i+1}$ is close to $A_{lp}$, and the max gap cannot occur between $A_{lp}$ and $A_{i+1}$ by our assumptions. So, update $A_{lp}$ to $A_{i+1}$. Updates may need to be cascaded—after one update to $A_{lp}$, consult the series of buckets up to and including $A_{lp}+G$ and update $A_{lp}$ to the largest number in those buckets.

I think the update cascade procedure may be correct—we cascade through each $A_i$ at most $O(1)$ times, and only move $A_{lp}$ if we’re sure that numbers before the new $A_{lp}$ cannot increase the max gap. However, among other things, we run into a problem after we’ve cascaded—it is now possible that $A_{i+1}$ occurs before $A_{lp}$ and in fact invalidates our $G$ and decreases the max gap—which is the same core problem we ran into before.

Intuitively, this make a lot of sense, and it is my error for not noticing it earlier. Again, we attempt to keep a constant amount of state per iteration, and again we are brought down by the possibility that we need to access a $O(N)$ history of max gaps in a certain iteration.

## Buckets

At this point I’m taking notice of the nice property of buckets, especially when we have $O(N)$ of them and can hash into them for free. In considering the expected initial gap for the previous approach, I also notice that the max gap is very rarely equal to $M/N$, and is usually much larger. We can determine if the max gap is equal to this by simply partitioning $M$ into $N$ buckets: if each bucket has one number, we are done, as we’ve essentially bucket sorted. otherwise, the max gap is greater than $M/N$.

In fact, we cannot have the max gap occur within any single bucket. So we don’t need all the numbers within every bucket, only two numbers for the min/max of the bucket. As the max gap cannot occur within a bucket, we need only check the max of each bucket with the min of the next bucket. So, we are done:

> Partition the range $K=A_{max}-A_{min}$ into $N$ buckets, and hash in $O(N)$ each number into its bucket. Track the min/max $B_i,C_i$ of each bucket and simply take the max of all $B_{i+1}-C_i$ and $C_i-B_i$ as the max gap.

## Reflection

The final solution is surprisingly simple and I am ashamed it took me so long to get it. I took a long twenty-minute detour the second time I considered iterative solutions, thought it did bring me marginally closer to the bucket solution. In hindsight, it should be immediately obvious based on the state information argument that any naive iterative solution is bound to fail.

Square root decomposition should have gotten me closer than it did. Any bucketing approach—both my solution and Mo’s algorithm—relies on the fact that buckets reduce the range of the problem. Though range doesn’t factor into the runtime here, range limits which numbers are relevant, and immediately decreases the state space of each bucket to constant.

In fact, I think bucketing solutions are likely quite flexible (I see them around a lot, e.g. in Bloom filters and VeB trees, if I recall correctly). The core intuition is that either the runtime or information state of a subproblem is tied to its range—and buckets reduce that range, essentially for free most of the time.

---

This problem took me around 40 minutes of focused airplane time to solve, which is a bit longer than I’d hope, considering the simplicity of the solution. I’ve been reflecting on this a bit—it’s not quite pride, nor honor, nor interview preparation, nor an interest in research, that seems to push me back to math and algorithms every so often. A mentor Richard once said to me:

> Suppose they put me in a room, and each time I solved the problem on screen, a green check mark would light up (this part is very important), and they would give me a bit of food. I would be pretty happy.

Well, Richard is a PhD (or postdoc (or professor?)) now. And I still can’t say I’ve found very many things that make me happier than those green check marks. I think, at some level, I’d much rather prefer working in the space of well-defined technical problems, than spaces which seem to smell like nepotism, privilege, appearance, and chance. But these are big words, and these spaces are big spaces whose boundaries are at least quite differentiable. I think I just want to feel in-control of my own destiny, and this problem made me feel that way.

---

I’ve been playing around with Github Copilot recently on recommendation of a friend Tony, and found it to be surprisingly compentent, but ultimately useless, while writing this post:

![](linear-max-gap.md-assets/2022-12-01-22-24-31.png)
*Close, Copilot.*

---

Sanjeev also mentions a variant of this problem, but for the min gap, which may require a randomized solution. I’ll tackle this later.

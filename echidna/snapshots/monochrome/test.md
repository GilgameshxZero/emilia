<!-- emilia-snapshot-properties
`silver` showcase
2025/03/20
monochrome test test-2
emilia-snapshot-properties -->

<!-- <input type="checkbox" class="silver-theme-toggle" enabled /> -->
<!-- <link rel="stylesheet" href="/silver/selective/high_contrast.css" /> -->

*This page showcases some typography options available in `silver` which is deployed with all snapshot-like media. Observe [this link to the repo](https://github.com/GilgameshxZero/silver) for further commentary.*

Snapshots are essays, deployed to emilia. They are tagged by a list of space-separated tags, then marked by a title and date, all in an HTML comment, ideally near the top of the file.

```c++
#include <iostream>

int main(int argc, char *argv[]) {
	// does not print hello world.
	std::cout << "this text should be syntax-highlighted" << std::endl;
	return 0;
}
```

# `h1` should see spacing before

and `h1` and `h2` headings can be subtitled by prepending a `div`. However, h1s are given subtitles by default in snapshots.

When printing, there should be a page break after this point. Also, ensure that `background graphics` are enabled so that code blocks, tables, and blockquotes are rendered correctly.

<div class="silver-page-break"></div>

<div class="silver-subtitled"></div>

## next, we’ll‘ observe these “curly quotes”

a dynamically requested subtitle via a `div` element, Aug. 24, 2021

and tables:

Function|Namespace
-|-
`cout`|`std`
forme|<pre><code>multi-line code blocks must be<br/>wrapped with HTML pre -> code and utilize br</code></pre>
*and now*|<blockquote>**a demonstration of images**<br/>test</blockquote>
<code>multi-line<br/>inline code</code>|$\LaTeX$<br/>$\begin{aligned}\text{can}&=\text{be put}\\\text{over}&=f(x^\text{multiple lines})\end{aligned}$<br/><br/>but will require inline HTML<br/><span class="katex-display">$f(x)=x^2-x+h$</span>As you may observe, `display`-style $\KaTeX$ may be used by wrapping the $\KaTeX$ syntax in a `span` element with class `katex-display`. Surely there is a way to add `tag`s too, but I have not yet figured it out.

![](test.md-assets/2021-08-24-14-16-53.avif#large)
*This should not be italicized, but styled as a caption.*

### Smaller titles

In some browsers, `autoplay` on a `video` element triggers fullscreen on a mobile device. We should be wary of this and only enable `autoplay` if this behavior is always desired, or case for mobile.

<video src="test.md-assets/emilia.mkv#large" autoplay loop muted></video>
*<i>Another caption, this time on a `webm` with some $\LaTeX$, manually italicized. Auto-italicization is easier when the italicization occurs not at the boundaries of the caption.</i>*

The CSS supports dark/light themes in VSCode webview preview. Upon printing to HTML, light/dark theme is set by system preference instead. This preference-dependence should be overwritten for purely light/dark themed websites.

#### an even smaller heading for $\LaTeX$

$\text{fonts in latex should be loaded at the same time as other fonts for markdown. longer lines in latex should render fine with scrollbars}$

things can be tagged by using `\tag` and $(1)$ as such

$$\int_0^\infty\sqrt{x}dx\tag{1}$$

equation 1 should have the square root display correctly (inline like $\sqrt{532}$ as well).

##### Display of square roots seem to depend on font-size for `katex`.

---

Lists and details/summary should be highlight to distinguish them from regular text.

* $3n+1$
* goldbach
* riemann

1. item 1
2. order number 2

> blockquotes should render fine. Consider how much spacing there should be around it.
>
> > a nested blockquote
> >
> > ```ts
> > console.log(`hello-world!`);
> > ```

###### Many more styling options exist. Check out `markdown.css` for more.

####### `h7` does not exist.

In particular pay attention to latex rendering, and task list rendering, which seem to use separate CSS.

* [ ] todo list 1
* [x] todo list 2
* [ ] todo list 3

Here is a test of the [snapshot interlinking framework](test-latin), which unfortunately does not work in local browser, but should work in VSCode & on-server.

<details>
<summary>

And a summary for $f(x)$ (LaTeX and inline code works if `<p>` display is changed to `inline`, which is default in `markdown.css`):

</summary>

The details!
Note that wrapped markdown also works: $f(x)=x$, as long as it is preceded by a blank line.

Unfortunately, there is currently (March 22, 2025) no way to auto-open all details during a print procedure: <https://github.com/w3c/csswg-drafts/issues/2084>.

</details>

# Fonts

A short font comparison.

<span class="kaiti">敏捷的棕色狐狸跳过了那只懒狗。[fangsong, kaiti]</span>
<span class="kaiti">*敏捷的棕色狐狸跳过了那只懒狗。[fangsong, kaiti]*</span>
<span class="kaiti">**敏捷的棕色狐狸跳过了那只懒狗。[fangsong, kaiti]**</span>
<span class="latin-modern-roman-10">The quick brown fox jumps over the lazy dog. [serif, latin-modern-roman-10]</span>
<span class="latin-modern-roman-10">*The quick brown fox jumps over the lazy dog. [serif, latin-modern-roman-10]*</span>
<span class="latin-modern-roman-10">**The quick brown fox jumps over the lazy dog. [serif, latin-modern-roman-10]**</span>
<span class="eb-garamond">The quick brown fox jumps over the lazy dog. [eb-garamond]</span>
<span class="eb-garamond">*The quick brown fox jumps over the lazy dog. [eb-garamond]*</span>
<span class="eb-garamond">**The quick brown fox jumps over the lazy dog. [eb-garamond]**</span>
<span class="roboto">The quick brown fox jumps over the lazy dog. [sans-serif, roboto]</span>
<span class="roboto">*The quick brown fox jumps over the lazy dog. [sans-serif, roboto]*</span>
<span class="roboto">**The quick brown fox jumps over the lazy dog. [sans-serif, roboto]**</span>
<span class="consolas">The quick brown fox jumps over the lazy dog. [monospace, consolas]</span>
<span class="consolas">*The quick brown fox jumps over the lazy dog. [monospace, consolas]*</span>
<span class="consolas">**The quick brown fox jumps over the lazy dog. [monospace, consolas]**</span>
<span class="tangerine">The quick brown fox jumps over the lazy dog. [cursive, tangerine]</span>
<span class="tangerine">*The quick brown fox jumps over the lazy dog. [cursive, tangerine]*</span>
<span class="tangerine">**The quick brown fox jumps over the lazy dog. [cursive, tangerine]**</span>

Some fonts have custom `size-adjust`s defined. You may observe them in `fonts.css`.

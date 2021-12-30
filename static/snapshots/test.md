This is a test page for `snapshot.css`.

```c++
#include <iostream>

int main(int argc, char *argv[]) {
	std::cout << "this text should be syntax-highlighted" << std::endl;
	return 0;
}
```

# `h1` should see spacing before

and `h1` and `h2` headings can be subtitled by prepending a `div`.

<div class="next-subtitled"></div>

## next, we’ll observe these curly quotes

a subtitle, Aug. 24, 2021

and tables:

| Function | Namespace                 |
| -------- | ------------------------- |
| `cout`   | `std`                     |
| forme    | forme has no name         |
| and now  | a demonstration of images |

![](test.md-assets/2021-08-24-14-16-53.jpg)
*This should not be italicized, but styled as a caption.*

### Smaller titles

<video src="test.md-assets/emilia.webm" autoplay loop muted></video>
*Another caption.*

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

###### Many more styling options exist. Check out `markdown.css` for more.

In particular pay attention to latex rendering, and task list rendering, which seem to use separate CSS.

* [ ] todo list 1
* [x] todo list 2
* [ ] todo list 3

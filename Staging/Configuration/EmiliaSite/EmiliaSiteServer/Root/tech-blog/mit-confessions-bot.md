# MIT Confessions Bot
Jun. 27, 2018

There are few things more human than language. Inspired by Karpathy's famous [RNN Post](http://karpathy.github.io/2015/05/21/rnn-effectiveness/) and my recent acquisition of a high-end GPU, I decided that it would be a good time to gain some hands-on RNN experience myself.

# Data

## Sources
Any ML projects begins with data acquisition. For this project, I chose the MIT Confessions Facebook pages as my source. Namely, there are three such pages:

* [MIT Confessions](https://www.facebook.com/beaverconfessions/)
* [MIT Timely Confessions](https://www.facebook.com/timelybeaverconfessions/)
* [MIT Summer Confessions](https://www.facebook.com/MITSummerConfessions/)

At one point in time, I did consider using confessions pages from other colleges - however, while this would allow us more data points, the increased diversity would surely make training the model more difficult. Moreover, I would lose anything which made MIT Confessions unique.

A brief look at our sources nets us some important observations:
* MIT
	* Most posts overall
	* Not active
	* Moderate amount of non-confessions announcements mixed in
* Timely
	* Moderate number of posts
	* Very active before summer
	* Very few announcements
* Summer
	* Very small number of posts
	* Currently very active
	* No announcements

## Acquisition
I am not familiar with Facebook's Graph API, and my latest interactions with it have proven difficult. As data acquisition will not be a common process, using Selenium here was likely a good choice.

Selenium offers the benefits of a very low learning curve as well as constant visualization. However, I still encountered a few problems at this step, which I solved over a few iterations:
* Infinite scroll: Facebook pages don't load all their posts at one time. I made several observations before I could look through all the posts while ensuring that I did not cause a tremendous slowdown by loading too many posts, or otherwise manually delay for longer than necessary to load posts.
* Expanding posts: Moderately long posts are initially collapsed. Many attempts to scrape posts using various attributes resulted in `...` appearing at the point Facebook decided to initially cut off the post. It took a while to realize that I had to manually expand the posts as well as find the right attribute to scrape.
	* Very long posts: As the frequency of these posts is low, I decided to forego the effort to scrape these.
* Non-text posts: Since we're working with RNNs here, simply discarding these posts was fine.

As a side, I also scraped the time at which posts were made. You can find the raw scraped data, in JSON, [here](tech-blog/assets/mit-confessions-bot/fetch.zip).

## Cleaning & Preparation
In preparation for training, I made a few design choices:
* Confession numbers not removed
	* Maximum Likelihood Language Model: Confession numbers were removed for this model
* Uppercase letters converted to lowercase
* Charset restricted to this regex: `[^a-z \n\'\.,?!0-9@#<>/:;\-\"()]`

You can find the parsed data as JSON [here](tech-blog/assets/mit-confessions-bot/parsed.zip). There are three versions of the text for each post:
* `orig-text`: Unmodified post text.
* `text`: Post text with the modifications above applied, but with the confession number still intact.
* `nr-text`: Same as `text` but without the confession number.

Before training, I also randomly concatenated the post texts together, separated by a special `NULL` character. [This](tech-blog/assets/mit-confessions-bot/data-mit-all-full.txt) is the concatenated file of posts with the confession number, and [this](tech-blog/assets/mit-confessions-bot/data-mit-all-nr.txt) is without.

# Char-RNN
## Research

## First Model: 256-256 LSTM

## Second Model: 512 LSTM

## Third Model: 512-512 GRU

## Temperature

## Samples

# Maximum Likelihood Lanugage Model

# Deployment
The page is live at [MIT Confessions Simulator](https://www.facebook.com/mitconfessionssimulator/).

# Final Thoughts
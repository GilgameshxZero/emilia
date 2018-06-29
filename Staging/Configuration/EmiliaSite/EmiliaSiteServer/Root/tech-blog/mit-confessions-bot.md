# MIT Confessions Bot
Jun. 27, 2018

There are few things more human than language. Inspired by Karpathy's famous [RNN Post](http://karpathy.github.io/2015/05/21/rnn-effectiveness/) and my recent acquisition of a high-end GPU, I decided that it would be a good time to gain some hands-on RNN experience myself.

# Overview
In this project, we will be training predictive languages model following the MIT Confessions Facebook pages. The model should be able to generate unlimited confessions posts given an initial seed sequence of text. In addition to a character-level RNN model (`char-RNN`), I also trained a char-based n-gram model (henceforth referred to as `char-n-gram`) which I will also contrast with the `char-RNN`.

In this post, I will focus on practical technicalities regarding the models rather than the mathematical background. There exist numerous resources which cover the latter.

# Data
## Sources
Any ML project begins with data acquisition. Namely, there are three such pages:

* [MIT Confessions](https://www.facebook.com/beaverconfessions/) (henceforth referred to as `mit`)
* [MIT Timely Confessions](https://www.facebook.com/timelybeaverconfessions/) (henceforth referred to as `timely`)
* [MIT Summer Confessions](https://www.facebook.com/MITSummerConfessions/) (henceforth referred to as `summer`)

At one point in time, I did consider using confessions pages from other colleges - however, while this would allow us more data points, the increased diversity would surely make training the model more difficult. Moreover, I would lose anything which made MIT Confessions unique.

A brief look at our sources nets us some important observations:
* `mit` has the most posts, then `timely`, then `summer`.
* Each has differing levels of current activity.
* There are varying amounts of non-confession announcements mixed in on each page.

## Acquisition (Scraping)
I am not familiar with Facebook's Graph API, and my latest interactions with it have proven difficult. As data acquisition will not be a common process, using Selenium here was likely a good choice. Selenium offers the benefits of a very low learning curve, as well as visualization of the scraping process. However, I still encountered a few problems at this step, which I solved over a few iterations:
* **Infinite scroll**: Facebook pages don't load all their posts at one time, but rather (mostly) chronologically. In order to reach the oldest posts, one must scroll down pretty far. Selenium doesn't have an easily accesible "wait for scroll to load" function, while a constant sleep duration would crash if there were connectivity hiccups and cause the process to be unnecessarily slow otherwise. Here's what I did:
	* Scrape posts as I scroll the page, and delete the post element once scraped: This prevents Chrome slowdowns from a very large DOM in memory.
	* The DOM id `www_pages_reaction_see_more_unitwww_pages_posts` only appears when there's more posts to load. So, scroll down while this id exists.
* **Long posts**: Moderately long posts are initially collapsed under a `See More` link. There are many attributes to each post which contain the post text, but most of them insert a `...` at the place of the `See More` link into the raw post text. The trick here is to first click the `See More` link (css selector `a.see_more_link`) and then scrape the `outerText` attribute of all `p` children of the post element, which avoids the `...` problem.
* **Extremely long posts**: These posts don't have a `See More` link but rather a `Continue Reading` link, which would open a new tab with the full post. I decided to disregard these posts, since they were few and far inbetween.
* **Non-text posts**: Since we're working with characters here, simply discarding these posts was fine.

As a side, I also scraped the time at which posts were made from the css selector `abbr._5ptz`. You can find the raw scraped data, in JSON, [here](tech-blog/assets/mit-confessions-bot/fetch.zip).

## Cleaning & Preparation
In preparation for training, I made a few design choices:
* **Confession numbers not removed**: Not too impactful in the long run; perhaps even beneficial to RNN training, considering that strings like `#1000` are strong indicators of a post beginning (beyond the special character to be added at post ends).
	* **char-n-gram**: Confession numbers were removed for this model, as they would most definitely make the model less robust.
* **Uppercase letters converted to lowercase**: I'm fairly certain this makes both the `char-RNN` and the `char-n-gram` easier to train, since it reduces the alphabet size. However, on the flip side, capitals are usually strongly favored after periods, so the char-RNN might learn
* **Charset restricted to this regex**: `[a-z \n\'\.,?!0-9@#<>/:;\-\"()]`. Again, this restricts the alphabet and would make the models easier to train.

The flipside of the latter two design choices above is that they restrict the alphabet of generated samples. In my case, this wasn't top priority to capture the intricacies of MIT Confessions language.

In order to have the model be able to write new posts, it is necessary for the model to be able to recognize post ends. Thus, I concatenated all cleaned posts into one large string to serve as the training data, each post being separated by a `\0` (NULL) character. Post order was randomized. [This](tech-blog/assets/mit-confessions-bot/data-mit-all-full.txt) is the concatenated file of posts with the confession number, and [this](tech-blog/assets/mit-confessions-bot/data-mit-all-nr.txt) is without. These strings served as training and validation data for both the `char-RNN` and the `char-n-gram`.

# Char-RNN
## Basics
* **Hardware**
	* CPU: Intel Core i7-7600U
	* GPU: Nvidia GeForce GTX 1080 Ti (external; Akitio Node TB3)
* **Framework**: Keras, Sequential Model

## First Model: 2x256 LSTM
### Layers
1. LSTM: 256
2. Dropout: 0.2
3. LSTM: 256
4. Dropout: 0.2
5. Softmax

### Other Specs
* **Optimizer**: `adam` (`lr` = 0.001)
* **Loss**: `categorical_crossentropy`
* **Input sequence length**: 50
* **Batch size**: 64

### Final Model
* **Weights**: [.hdf5]()
* **Loss**: ~1.530
* **Epochs**: 43 out of 50 planned

### Sampling
* Did not incorporate temperature (by default, temperature = 1).
* Using `argmax` instead of choosing from output distribution caused repetitive patterns in samples.

### Thoughts
* Small batch sizes slowed down training.
* Loss declines plateaued after epoch 40.

## Second Model: 1x512 LSTM
### Layers
1. LSTM: 512
2. Dropout: 0.4
3. Softmax

### Other Specs
* **Optimizer**: `adam` (`lr` = 0.001, `lr`=0.00001 after epoch 18)
* **Loss**: `categorical_crossentropy`
* **Input sequence length**: 150
* **Batch size**: 256

### Final Model
* **Weights**: [.hdf5]()
* **Loss**: ~1.681
* **Epochs**: 22 out of 100 planned

## Third Model: 2x512 GRU
### Layers
1. GRU: 512
2. BatchNorm
3. Dropout: 0.4
1. GRU: 512
2. BatchNorm
3. Dropout: 0.4
4. Softmax

### Other Specs
* **Optimizer**: `adam` (`lr` = 0.0003 (Karpathy constant))
* **Loss**: `categorical_crossentropy`
* **Input sequence length**: 100
* **Batch size**: 256

### Final Model
* **Weights**: [.hdf5]()
* **Loss**: ~1.412
* **Epochs**: 46 out of 100 planned

## Temperature
Temperature is a layer added before the final `softmax` layer during sampling, which condenses or expands the distribution of pre-softmax activations. In Keras, I used a `Lambda` layer. Lower temperatures lead to more consistent, less creative samples, and higher temperatures lead to a more confident model and more experimental samples.

## Samples

# Char-N-Gram

# Deployment
The page is live at [MIT Confessions Simulator](https://www.facebook.com/mitconfessionssimulator/).

# Final Thoughts
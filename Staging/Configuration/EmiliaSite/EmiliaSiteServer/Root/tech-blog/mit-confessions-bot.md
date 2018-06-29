# Predictive Text RNNs for MIT Confessions
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

At one point in time, I did consider using confessions pages from other colleges - however, while this would allow us more datapoints, the increased diversity would surely make training the model more difficult. Moreover, I would lose anything which made MIT Confessions unique.

A brief look at our sources nets us some important observations:
* `mit` has the most posts, then `timely`, then `summer`.
* Each has differing levels of current activity.
* There are varying amounts of non-confession announcements mixed in on each page.

## Acquisition (Scraping)
I am not familiar with Facebook's Graph API, and my latest interactions with it have proven difficult. As data acquisition will not be a common process, using Selenium here was likely a good choice. Selenium offers the benefits of a very low learning curve, as well as visualization of the scraping process. However, I still encountered a few problems at this step, which I solved over a few iterations:
* **Infinite scroll**: Facebook pages don't load all their posts at one time, but rather (mostly) chronologically. In order to reach the oldest posts, one must scroll down pretty far. Selenium doesn't have an easily accessible "wait for scroll to load" function, while a constant sleep duration would crash if there were connectivity hiccups and cause the process to be unnecessarily slow otherwise. Here's what I did:
	* Scrape posts as I scroll the page, and delete the post element once scraped: This prevents Chrome slowdowns from a very large DOM in memory.
	* The DOM id `www_pages_reaction_see_more_unitwww_pages_posts` only appears when there's more posts to load. So, scroll down while this id exists.
* **Long posts**: Moderately long posts are initially collapsed under a `See More` link. There are many attributes to each post which contain the post text, but most of them insert a `...` at the place of the `See More` link into the raw post text. The trick here is to first click the `See More` link (css selector `a.see_more_link`) and then scrape the `outerText` attribute of all `p` children of the post element, which avoids the `...` problem.
* **Extremely long posts**: These posts don't have a `See More` link but rather a `Continue Reading` link, which would open a new tab with the full post. I decided to disregard these posts, since they were few and far in-between.
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
	* GPU: Nvidia GeForce GTX 1080 Ti (external; Akitio Node; TB3 w/ 2 PCIe lanes)
		* eGPU bandwidth did not appear to be a bottleneck; raising batch sizes (under memory limits) still resulted in faster epoch training times.
	* Memory: 16GB
* **Framework**: Keras, Sequential Model

### Training, Validation, and Test Data
Given the input dimension (henceforth referred to as `sequence length`) of the network, datapoints were generated by sliding a window of length `sequence length + 1` over the input string from the last section, with the  `sequence length` prefix serving as input and the final character in the window serving as the label.

None of the models used test datasets, as is customary of RNNs.

The first two models did not use validation sets; however, the third model repurposed `10%` of the training data as validation. The validation data was chosen randomly from all the training datapoints. In hindsight, this would not prevent overfitting very well, as most of the training data inputs would overlap in some way with the validation data inputs due to the nature of the sliding window method of datapoint generation.

Data from all three sources combined totalled just over 1MB.

## Temperature
[Temperature](https://cs.stackexchange.com/questions/79241/what-is-temperature-in-lstm-and-neural-networks-generally) is implemented as a `Lambda` layer before the final `softmax` layer during sampling, which condenses or expands the distribution of pre-softmax activations. Lower temperatures lead to more consistent, less creative samples, and higher temperatures lead to a more confident model and more experimental samples.

Temperature was not implemented during sampling for the first model.

## Sampling
Samples require a seed sequence of a specified length. Seeds were chosen as random contiguous. sequences of the training data for all three models.

Each `char-RNN` model outputs a distribution of probabilities for each of the 55 characters in our alphabet as its prediction for the next character. The first model used `argmax` on this distribution to predict the next character, which resulted in very repetitive patters. Sampling in later models simply picked from the distribution itself (`numpy.random.choice`) which results in more realistic samples.

## First Model: 2x256 LSTM
### Layers
1. LSTM: 256
2. Dropout: 0.2
3. LSTM: 256
4. Dropout: 0.2
5. Softmax

Largely based on [this](https://machinelearningmastery.com/text-generation-lstm-recurrent-neural-networks-python-keras/) post.

### Other Specs
* **Optimizer**: `adam` (`lr` = 0.001)
* **Loss**: `categorical_crossentropy`
* **Input sequence length**: 50
	* This is both the length of the seed text for sampling as well as the input dimension into the network.
* **Batch size**: 64
	* Small batch sizes slowed down training when compared to later models. However, smaller batch sizes are less prone to overfitting while large batch sizes might lead loss to converge at a higher value than the true minimum.
* **Validation**: None

### Final Model
* **Weights**: [.hdf5]()
* **Loss**: ~1.530
	* Loss plateaued around after epoch 40 - it appeared that we had reached convergence.
* **Epochs**: 43 out of 50 planned
* **Samples**: [1000 characters]()

### Thoughts
Samples generated from this first model were largely repetitive due to the lack of temperature and using `argmax` during sampling. However, this does not mitigate the concern that loss plateaued at a high value. Given the small batch size and the high loss, I should have realized this was an indicator that the network was not big enough. Instead, I stupidly made the network more shallow instead for the second model.

## Second Model: 1x512 LSTM
### Layers
1. LSTM: 512
2. Dropout: 0.4
3. Softmax

I thought perhaps the high loss was due to the 2-layer network being too difficult to train, so for the second model, I (mistakenly) reduced the network to just one layer.

I don't believe dropout affects loss minima, but only the time required to reach it. In smaller models with just `summer` data, it seemed like a dropout of 0.4 resulted in the fastest loss convergences.

### Other Specs
* **Optimizer**: `adam` (`lr` = 0.001, `lr` = 0.00001 after epoch 18)
	* Loss plateaued at ~1.814 around epoch 18. Learning rates too high might lead to loss convergence at a value higher than the true minima, which learning rates too low will still converge to the true minima but slower. Since loss was already plateauing, I decided to lower the learning rate in case we were not approaching the true minima. In hindsight, this was the right move.
* **Loss**: `categorical_crossentropy`
* **Input sequence length**: 150
	* Input sequence length was increased from the last model mainly because Karpathy's RNN mentioned using a sequence length of 100, and 150 would not hurt the potential of the model but only training speed. Moreover, larger sequence lengths (network input dimensions) would utilize GPU parallel processing better, and would not slow training down by that much in this case.
* **Batch size**: 256
	* Batch size was increased to speed up training at the risk of overfitting. Batch sizes larger than 256 did not offer significant speedups to training.
* **Validation**: None
	* It would have been helpful to split the training data into training and validation data, especially to prevent overfitting with the larger batch size; however, I did not do this.

### Final Model
* **Weights**: [.hdf5]()
* **Loss**: ~1.681
* **Epochs**: 22 out of 100 planned
* **Samples**: [1000 characters]()

### Thoughts
Unfortunately, loss seemed to converge at a higher number for this model than the last. Samples generated were significantly better, however, with the incorporation of temperature and choosing from the output distribution. I found `0.35` to be a solid temperature choice for the final iteration of this model.

## Third Model: 2x512 GRU
Feeling a bit stumped after the last model, I gave my friend **Tony** a call and he offered me a few hyperparameter suggestions for this third model regarding GRUs, batch normalization, and learning rates for Adam.

### Layers
1. GRU: 512
2. BatchNorm
3. Dropout: 0.4
1. GRU: 512
2. BatchNorm
3. Dropout: 0.4
4. Softmax

This network is modeled after Karpathy's Paul Graham RNN model, with the addition of batch normalization and dropouts and GRU layers rather than LSTM. It's commonly assumed that batch normalization is a low-cost way to prevent internal covariate shift; however, recent research suggests otherwise. Regardless, batch normalization is considered non-harmful and usually helpful.

I am not familiar with the mathematics behind GRUs; however, research suggests that they do not lower the potential of a network compared to LSTMs while being much faster to train. This was indeed the case, as this two-layer network trained just as fast as the one-layer network from the second model.

### Other Specs
* **Optimizer**: `adam` (`lr` = 0.0003 ([Karpathy constant](https://twitter.com/karpathy/status/801621764144971776?lang=en)))
	* Seems like `3e-4` is the commonly used learning rate for `adam` as opposed to the original paper's suggestion of `0.001`.
* **Loss**: `categorical_crossentropy`
* **Input sequence length**: 100
	* Lowered from previous model to speed up training; impact is probably negligible.
* **Batch size**: 256
* **Validation**: 10% of training datapoints
	* Due to the correlation between training and validation datapoints, validation loss probably was not a very good indicator of overfitting. However, some validation is better than none.

### Final Model
* **Weights**: [.hdf5]()
* **Loss**: ~1.412
	* Loss was nearing convergence, but likely still had a bit to go. Unfortunately, validation loss was not following, suggesting an overfitted model. However, sampling suggests otherwise. I should have kept on training, but I ran out of patience.
* **Epochs**: 46 out of 100 planned
* **Samples**: [1000 characters]()

### Thoughts

# Char-N-Gram
Directly implemented from [this](http://nbviewer.jupyter.org/gist/yoavg/d76121dfde2618422139) post.

# Deployment
The page is live at [MIT Confessions Simulator](https://www.facebook.com/mitconfessionssimulator/).

# Final Thoughts
[Github repo w/ source code]()

Predictive text is deceptively difficult
# IsoTree

:evergreen_tree: [IsoTree](https://github.com/david-cortes/isotree) - outlier/anomaly detection using Isolation Forest - for Ruby

Learn how [Isolation Forest](https://www.youtube.com/watch?v=RyFQXQf4w4w) works

:deciduous_tree: Check out [OutlierTree](https://github.com/ankane/outliertree) for human-readable explanations of outliers

[![Build Status](https://travis-ci.org/ankane/isotree.svg?branch=master)](https://travis-ci.org/ankane/isotree)

## Installation

Add this line to your application’s Gemfile:

```ruby
gem 'isotree'
```

## Getting Started

Prep your data

```ruby
data = [
  {department: "Books",  sale: false, price: 2.50},
  {department: "Books",  sale: true,  price: 3.00},
  {department: "Movies", sale: false, price: 5.00}
]
```

Train a model

```ruby
model = IsoTree::IsolationForest.new
model.fit(data)
```

Get outlier scores

```ruby
model.predict(data)
```

Scores are between 0 and 1, with higher scores indicating outliers

## Parameters

Pass parameters - default values below

```ruby
IsoTree::IsolationForest.new(
  sample_size: nil,
  ntrees: 500,
  ndim: 3,
  ntry: 3,
  prob_pick_avg_gain: 0,
  prob_pick_pooled_gain: 0,
  prob_split_avg_gain: 0,
  prob_split_pooled_gain: 0,
  min_gain: 0,
  all_perm: false,
  coef_by_prop: false,
  sample_with_replacement: false,
  penalize_range: true,
  weigh_by_kurtosis: false,
  min_imp_obs: 3,
  random_seed: 1,
  nthreads: -1
)
```

See a [detailed explanation](https://isotree.readthedocs.io/en/latest/#isotree.IsolationForest)

## Data

Data can be an array of hashes [master]

```ruby
[
  {department: "Books",  sale: false, price: 2.50},
  {department: "Books",  sale: true,  price: 3.00},
  {department: "Movies", sale: false, price: 5.00}
]
```

Or a Rover data frame [master]

```ruby
Rover.read_csv("data.csv")
```

Or a Numo array

```ruby
Numo::NArray.cast([[1, 2, 3], [4, 5, 6]])
```

## Performance

IsoTree uses OpenMP when possible for best performance. To enable OpenMP on Mac, run:

```sh
brew install libomp
```

Then reinstall the gem.

```sh
gem uninstall isotree --force
bundle install
```

## Reference

Get the average isolation depth [master]

```ruby
model.predict(data, output: "avg_depth")
```

## History

View the [changelog](https://github.com/ankane/isotree/blob/master/CHANGELOG.md)

## Contributing

Everyone is encouraged to help improve this project. Here are a few ways you can help:

- [Report bugs](https://github.com/ankane/isotree/issues)
- Fix bugs and [submit pull requests](https://github.com/ankane/isotree/pulls)
- Write, clarify, or fix documentation
- Suggest or add new features

To get started with development:

```sh
git clone --recursive https://github.com/ankane/isotree.git
cd isotree
bundle install
bundle exec rake compile
bundle exec rake test
```

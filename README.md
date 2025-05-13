# IsoTree Ruby

:evergreen_tree: [IsoTree](https://github.com/david-cortes/isotree) - outlier/anomaly detection using Isolation Forest - for Ruby

Learn how [Isolation Forest](https://www.youtube.com/watch?v=RyFQXQf4w4w) works

:deciduous_tree: Check out [OutlierTree](https://github.com/ankane/outliertree-ruby) for human-readable explanations of outliers

[![Build Status](https://github.com/ankane/isotree-ruby/actions/workflows/build.yml/badge.svg)](https://github.com/ankane/isotree-ruby/actions)

## Installation

Add this line to your application’s Gemfile:

```ruby
gem "isotree"
```

Windows is not supported at the moment

## Getting Started

Prep your data

```ruby
data = [
  {department: "Books",  sale: false, price: 2.50},
  {department: "Books",  sale: true,  price: 3.00},
  {department: "Movies", sale: false, price: 5.00},
  # ...
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

Export the model

```ruby
model.export_model("model.bin")
```

Import a model

```ruby
model = IsoTree::IsolationForest.import_model("model.bin")
```

## Parameters

Pass parameters - default values below

```ruby
IsoTree::IsolationForest.new(
  sample_size: "auto",
  ntrees: 500,
  ndim: 3,
  ntry: 1,
  max_depth: "auto",
  ncols_per_tree: nil,
  prob_pick_pooled_gain: 0.0,
  prob_pick_avg_gain: 0.0,
  prob_pick_full_gain: 0.0,
  prob_pick_dens: 0.0,
  prob_pick_col_by_range: 0.0,
  prob_pick_col_by_var: 0.0,
  prob_pick_col_by_kurt: 0.0,
  min_gain: 0.0,
  missing_action: "auto",
  new_categ_action: "auto",
  categ_split_type: "auto",
  all_perm: false,
  coef_by_prop: false,
  sample_with_replacement: false,
  penalize_range: false,
  standardize_data: true,
  scoring_metric: "depth",
  fast_bratio: true,
  weigh_by_kurtosis: false,
  coefs: "uniform",
  assume_full_distr: true,
  min_imp_obs: 3,
  depth_imp: "higher",
  weigh_imp_rows: "inverse",
  random_seed: 1,
  use_long_double: false,
  nthreads: -1
)
```

See a [detailed explanation](https://isotree.readthedocs.io/en/latest/#isotree.IsolationForest)

## Data

Data can be an array of hashes

```ruby
[
  {department: "Books",  sale: false, price: 2.50},
  {department: "Books",  sale: true,  price: 3.00},
  {department: "Movies", sale: false, price: 5.00}
]
```

Or a Rover data frame

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

## Deployment

Check out [Trove](https://github.com/ankane/trove) for deploying models.

```sh
trove push model.bin
```

## Reference

Get the average isolation depth

```ruby
model.predict(data, output: "avg_depth")
```

## History

View the [changelog](https://github.com/ankane/isotree-ruby/blob/master/CHANGELOG.md)

## Contributing

Everyone is encouraged to help improve this project. Here are a few ways you can help:

- [Report bugs](https://github.com/ankane/isotree-ruby/issues)
- Fix bugs and [submit pull requests](https://github.com/ankane/isotree-ruby/pulls)
- Write, clarify, or fix documentation
- Suggest or add new features

To get started with development:

```sh
git clone --recursive https://github.com/ankane/isotree-ruby.git
cd isotree-ruby
bundle install
bundle exec rake compile
bundle exec rake test
```

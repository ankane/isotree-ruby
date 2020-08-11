module IsoTree
  class IsolationForest
    def initialize(
      sample_size: nil, ntrees: 500, ndim: 3, ntry: 3,
      prob_pick_avg_gain: 0, prob_pick_pooled_gain: 0,
      prob_split_avg_gain: 0, prob_split_pooled_gain: 0,
      min_gain: 0, all_perm: false, coef_by_prop: false,
      sample_with_replacement: false, penalize_range: true,
      weigh_by_kurtosis: false, min_imp_obs: 3, random_seed: 1, nthreads: -1
    )

      @sample_size = sample_size
      @ntrees = ntrees
      @ndim = ndim
      @ntry = ntry
      @prob_pick_avg_gain = prob_pick_avg_gain
      @prob_pick_pooled_gain = prob_pick_pooled_gain
      @prob_split_avg_gain = prob_split_avg_gain
      @prob_split_pooled_gain = prob_split_pooled_gain
      @min_gain = min_gain
      @all_perm = all_perm
      @coef_by_prop = coef_by_prop
      @sample_with_replacement = sample_with_replacement
      @penalize_range = penalize_range
      @weigh_by_kurtosis = weigh_by_kurtosis
      @min_imp_obs = min_imp_obs
      @random_seed = random_seed

      # etc module returns virtual cores
      nthreads = Etc.nprocessors if nthreads < 0
      @nthreads = nthreads
    end

    def fit(x)
      options = data_options(x).merge(fit_options)
      options[:sample_size] ||= options[:nrows]
      @ncols = options[:ncols]
      @ext_iso_forest = Ext.fit_iforest(options)
    end

    def predict(x)
      raise "Not fit" unless @ext_iso_forest
      options = data_options(x).merge(nthreads: @nthreads)
      if options[:ncols] != @ncols
        raise ArgumentError, "Input must have #{@ncols} columns for this model"
      end
      Ext.predict_iforest(@ext_iso_forest, options)
    end

    private

    # TODO support categorical data
    def data_options(x)
      if defined?(Numo::NArray) && x.is_a?(Numo::NArray)
        raise ArgumentError, "Input must have 2 dimensions" if x.ndim != 2
        x = x.cast_to(Numo::DFloat)
        nrows, ncols = x.shape
        numeric_data = String.new
        ncols.times do |i|
          numeric_data << x[true, i].to_binary
        end
      else
        x = x.to_a
        nrows = x.size
        ncols = x.first ? x.first.size : 0
        if x.any? { |r| r.size != ncols }
          raise ArgumentError, "All rows must have the same number of columns"
        end
        numeric_data = String.new
        ncols.times do |i|
          numeric_data << x.map { |v| v[i] }.pack("d*")
        end
      end
      raise ArgumentError, "No data" if nrows == 0

      {
        nrows: nrows,
        ncols: ncols,
        numeric_data: numeric_data
      }
    end

    def fit_options
      keys = %i(
        sample_size ntrees ndim ntry
        prob_pick_avg_gain prob_pick_pooled_gain
        prob_split_avg_gain prob_split_pooled_gain
        min_gain all_perm coef_by_prop
        sample_with_replacement penalize_range
        weigh_by_kurtosis min_imp_obs
        random_seed nthreads
      )
      options = {}
      keys.each do |k|
        options[k] = instance_variable_get("@#{k}")
      end
      options
    end
  end
end

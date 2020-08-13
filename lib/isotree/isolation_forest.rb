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
      x = Dataset.new(x)
      prep_fit(x)
      options = data_options(x).merge(fit_options)
      options[:sample_size] ||= options[:nrows]
      @ext_iso_forest = Ext.fit_iforest(options)
    end

    def predict(x)
      raise "Not fit" unless @ext_iso_forest

      x = Dataset.new(x)
      prep_predict(x)
      options = data_options(x).merge(nthreads: @nthreads)
      Ext.predict_iforest(@ext_iso_forest, options)
    end

    private

    def prep_fit(df)
      @numeric_columns = df.numeric_columns
      @categorical_columns = df.categorical_columns
      @categories = {}
      @categorical_columns.each do |k|
        @categories[k] = df[k].uniq.to_a.compact.map.with_index.to_h
      end
    end

    # TODO handle column type mismatches
    def prep_predict(df)
      expected_columns = @numeric_columns + @categorical_columns
      if df.array_type
        if df.numeric_columns.size + df.categorical_columns.size != expected_columns.size
          raise ArgumentError, "Input must have #{expected_columns.size} columns for this model"
        end
      end
      expected_columns.each do |k|
        raise ArgumentError, "Missing column: #{k}" unless df[k]
      end
    end

    def data_options(df)
      options = {}

      # numeric
      numeric_data = String.new
      @numeric_columns.each do |k|
        v = df[k]
        v = v.to_numo if v.respond_to?(:to_numo) # Rover
        binary_str =
          if v.respond_to?(:to_binary) # Rover and Numo
            v.cast_to(Numo::DFloat).to_binary
          else
            v.pack("d*")
          end
        numeric_data << binary_str
      end
      options[:numeric_data] = numeric_data
      options[:ncols_numeric] = @numeric_columns.size

      # categorical
      categorical_data = String.new
      ncat = String.new
      @categorical_columns.each do |k|
        categories = @categories[k]
        # for unseen values, set to categories.size
        categories_size = categories.size
        values = df[k].map { |v| v.nil? ? -1 : (categories[v] || categories_size) }
        # TODO make more efficient
        if values.any? { |v| v == categories_size }
          warn "[isotree] Unseen values in column: #{k}"
        end

        v = values
        v = v.to_numo if v.respond_to?(:to_numo) # Rover
        binary_str =
          if v.respond_to?(:to_binary) # Rover and Numo
            v.cast_to(Numo::Int32).to_binary
          else
            v.pack("i*")
          end
        categorical_data << binary_str
        ncat << [categories.size].pack("i")
      end
      options[:categorical_data] = categorical_data
      options[:ncols_categ] = @categorical_columns.size
      options[:ncat] = ncat

      options[:nrows] = df.size
      options
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

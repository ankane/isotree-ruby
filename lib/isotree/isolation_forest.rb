module IsoTree
  class IsolationForest
    def initialize(
      sample_size: nil, ntrees: 500, ndim: 3, ntry: 3,
      prob_pick_avg_gain: 0, prob_pick_pooled_gain: 0,
      prob_split_avg_gain: 0, prob_split_pooled_gain: 0,
      min_gain: 0, missing_action: "impute", new_categ_action: "smallest",
      categ_split_type: "subset", all_perm: false, coef_by_prop: false,
      sample_with_replacement: false, penalize_range: true,
      weigh_by_kurtosis: false, coefs: "normal", min_imp_obs: 3, depth_imp: "higher",
      weigh_imp_rows: "inverse", random_seed: 1, nthreads: -1
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
      @missing_action = missing_action
      @new_categ_action = new_categ_action
      @categ_split_type = categ_split_type
      @all_perm = all_perm
      @coef_by_prop = coef_by_prop
      @sample_with_replacement = sample_with_replacement
      @penalize_range = penalize_range
      @weigh_by_kurtosis = weigh_by_kurtosis
      @coefs = coefs
      @min_imp_obs = min_imp_obs
      @depth_imp = depth_imp
      @weigh_imp_rows = weigh_imp_rows
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

    def predict(x, output: "score")
      check_fit

      x = Dataset.new(x)
      prep_predict(x)

      options = data_options(x).merge(nthreads: @nthreads)
      case output
      when "score"
        options[:standardize] = true
      when "avg_depth"
        options[:standardize] = false
      else
        raise ArgumentError, "Unknown output"
      end

      Ext.predict_iforest(@ext_iso_forest, options)
    end

    # same format as Python so models are compatible
    def export_model(path)
      check_fit

      File.write("#{path}.metadata", JSON.pretty_generate(export_metadata))
      Ext.serialize_ext_isoforest(@ext_iso_forest, path)
    end

    def self.import_model(path)
      model = new
      metadata = JSON.parse(File.read("#{path}.metadata"))
      model.send(:import_metadata, metadata)
      model.instance_variable_set(:@ext_iso_forest, Ext.deserialize_ext_isoforest(path))
      model
    end

    private

    def export_metadata
      data_info = {
        ncols_numeric: @numeric_columns.size,
        ncols_categ: @categorical_columns.size,
        cols_numeric: @numeric_columns,
        cols_categ: @categorical_columns,
        cat_levels: @categorical_columns.map { |v| @categories[v].keys }
      }

      # Ruby-specific
      data_info[:sym_numeric] = @numeric_columns.map { |v| v.is_a?(Symbol) }
      data_info[:sym_categ] = @categorical_columns.map { |v| v.is_a?(Symbol) }

      model_info = {
        ndim: @ndim,
        nthreads: @nthreads,
        build_imputer: false
      }

      params = {}
      PARAM_KEYS.each do |k|
        params[k] = instance_variable_get("@#{k}")
      end

      {
        data_info: data_info,
        model_info: model_info,
        params: params
      }
    end

    def import_metadata(metadata)
      data_info = metadata["data_info"]
      model_info = metadata["model_info"]
      params = metadata["params"]

      # Ruby-specific
      sym_numeric = data_info["sym_numeric"].to_a
      sym_categ = data_info["sym_categ"].to_a

      @numeric_columns = data_info["cols_numeric"].map.with_index { |v, i| sym_numeric[i] ? v.to_sym : v }
      @categorical_columns = data_info["cols_categ"].map.with_index { |v, i| sym_categ[i] ? v.to_sym : v }
      @categories = {}
      @categorical_columns.zip(data_info["cat_levels"]) do |col, levels|
        @categories[col] = levels.map.with_index.to_h
      end

      @ndim = model_info["ndim"]
      @nthreads = model_info["nthreads"]

      PARAM_KEYS.each do |k|
        instance_variable_set("@#{k}", params[k.to_s])
      end
    end

    def check_fit
      raise "Not fit" unless @ext_iso_forest
    end

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

    PARAM_KEYS = %i(
      sample_size ntrees ntry max_depth
      prob_pick_avg_gain prob_pick_pooled_gain
      prob_split_avg_gain prob_split_pooled_gain min_gain
      missing_action new_categ_action categ_split_type coefs depth_imp
      weigh_imp_rows min_imp_obs random_seed all_perm coef_by_prop
      weights_as_sample_prob sample_with_replacement penalize_range
      weigh_by_kurtosis assume_full_distr
    )

    def fit_options
      keys = %i(
        sample_size ntrees ndim ntry
        prob_pick_avg_gain prob_pick_pooled_gain
        prob_split_avg_gain prob_split_pooled_gain
        min_gain missing_action new_categ_action
        categ_split_type all_perm coef_by_prop
        sample_with_replacement penalize_range
        weigh_by_kurtosis coefs min_imp_obs depth_imp
        weigh_imp_rows random_seed nthreads
      )
      options = {}
      keys.each do |k|
        options[k] = instance_variable_get("@#{k}")
      end
      options
    end
  end
end

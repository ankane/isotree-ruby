module IsoTree
  class IsolationForest
    def initialize(
      sample_size: "auto",
      ntrees: 500,
      ndim: 3,
      ntry: 1,
      # categ_cols: nil,
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
      # recode_categ: false,
      weights_as_sample_prob: true,
      sample_with_replacement: false,
      penalize_range: false,
      standardize_data: true,
      scoring_metric: "depth",
      fast_bratio: true,
      weigh_by_kurtosis: false,
      coefs: "uniform",
      assume_full_distr: true,
      # build_imputer: false,
      min_imp_obs: 3,
      depth_imp: "higher",
      weigh_imp_rows: "inverse",
      random_seed: 1,
      use_long_double: false,
      nthreads: -1
    )
      @sample_size = sample_size
      @ntrees = ntrees
      @ndim = ndim
      @ntry = ntry
      # @categ_cols = categ_cols
      @max_depth = max_depth
      @ncols_per_tree = ncols_per_tree
      @prob_pick_pooled_gain = prob_pick_pooled_gain
      @prob_pick_avg_gain = prob_pick_avg_gain
      @prob_pick_full_gain = prob_pick_full_gain
      @prob_pick_dens = prob_pick_dens
      @prob_pick_col_by_range = prob_pick_col_by_range
      @prob_pick_col_by_var = prob_pick_col_by_var
      @prob_pick_col_by_kurt = prob_pick_col_by_kurt
      @min_gain = min_gain
      @missing_action = missing_action
      @new_categ_action = new_categ_action
      @categ_split_type = categ_split_type
      @all_perm = all_perm
      @coef_by_prop = coef_by_prop
      # @recode_categ = recode_categ
      @weights_as_sample_prob = weights_as_sample_prob
      @sample_with_replacement = sample_with_replacement
      @penalize_range = penalize_range
      @standardize_data = standardize_data
      @scoring_metric = scoring_metric
      @fast_bratio = fast_bratio
      @weigh_by_kurtosis = weigh_by_kurtosis
      @coefs = coefs
      @assume_full_distr = assume_full_distr
      @min_imp_obs = min_imp_obs
      @depth_imp = depth_imp
      @weigh_imp_rows = weigh_imp_rows
      @random_seed = random_seed
      @use_long_double = use_long_double

      # etc module returns virtual cores
      nthreads = Etc.nprocessors if nthreads < 0
      @nthreads = nthreads
    end

    def fit(x)
      # make export consistent with Python library
      update_params

      x = Dataset.new(x)
      prep_fit(x)
      options = data_options(x).merge(fit_options)

      if options[:sample_size] == "auto"
        options[:sample_size] = [options[:nrows], 10000].min
      end

      # prevent segfault
      options[:sample_size] = options[:nrows] if options[:sample_size] > options[:nrows]

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
    def export_model(path, add_metada_file: false)
      check_fit

      metadata = export_metadata
      if add_metada_file
        # indent 4 spaces like Python
        File.write("#{path}.metadata", JSON.pretty_generate(metadata, indent: "    "))
      end
      Ext.serialize_combined(@ext_iso_forest, path, JSON.generate(metadata))
    end

    def self.import_model(path)
      model = new
      ext_iso_forest, metadata = Ext.deserialize_combined(path)
      model.instance_variable_set(:@ext_iso_forest, ext_iso_forest)
      model.send(:import_metadata, JSON.parse(metadata))
      model
    end

    private

    def export_metadata
      data_info = {
        ncols_numeric: @numeric_columns.size,
        ncols_categ: @categorical_columns.size,
        cols_numeric: @numeric_columns,
        cols_categ: @categorical_columns,
        cat_levels: @categorical_columns.map { |v| @categories[v].keys },
        categ_cols: [],
        categ_max: []
      }

      # Ruby-specific
      data_info[:sym_numeric] = @numeric_columns.map { |v| v.is_a?(Symbol) }
      data_info[:sym_categ] = @categorical_columns.map { |v| v.is_a?(Symbol) }

      model_info = {
        ndim: @ndim,
        nthreads: @nthreads,
        use_long_double: @use_long_double,
        build_imputer: false
      }

      params = {}
      PARAM_KEYS.each do |k|
        params[k] = instance_variable_get("@#{k}")
      end

      if params[:max_depth] == "auto"
        params[:max_depth] = 0
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
      @use_long_double = model_info["use_long_double"]
      @build_imputer = model_info["build_imputer"]

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
      sample_size ntrees ntry max_depth ncols_per_tree
      prob_pick_avg_gain prob_pick_pooled_gain prob_pick_full_gain prob_pick_dens
      prob_pick_col_by_range prob_pick_col_by_var prob_pick_col_by_kurt
      min_gain missing_action new_categ_action categ_split_type coefs
      depth_imp weigh_imp_rows min_imp_obs random_seed all_perm
      coef_by_prop weights_as_sample_prob sample_with_replacement penalize_range standardize_data
      scoring_metric fast_bratio weigh_by_kurtosis assume_full_distr
    )

    def fit_options
      keys = %i(
        sample_size ntrees ndim ntry
        categ_cols max_depth ncols_per_tree
        prob_pick_pooled_gain prob_pick_avg_gain
        prob_pick_full_gain prob_pick_dens
        prob_pick_col_by_range prob_pick_col_by_var prob_pick_col_by_kurt
        min_gain missing_action new_categ_action
        categ_split_type all_perm coef_by_prop
        weights_as_sample_prob
        sample_with_replacement penalize_range standardize_data
        scoring_metric fast_bratio
        weigh_by_kurtosis coefs min_imp_obs depth_imp
        weigh_imp_rows random_seed use_long_double nthreads
      )
      options = {}
      keys.each do |k|
        options[k] = instance_variable_get("@#{k}")
      end

      if options[:max_depth] == "auto"
        options[:max_depth] = 0
        options[:limit_depth] = true
      end

      if options[:ncols_per_tree].nil?
        options[:ncols_per_tree] = 0
      end

      options
    end

    def update_params
      if @missing_action == "auto"
        if @ndim == 1
          @missing_action = "divide"
        else
          @missing_action = "impute"
        end
      end

      if @new_categ_action == "auto"
        if @ndim == 1
          @new_categ_action = "weighted"
        else
          @new_categ_action = "impute"
        end
      end

      if @categ_split_type == "auto"
        if @ndim == 1
          @categ_split_type = "single_categ"
        else
          @categ_split_type = "subset"
        end
      end
    end
  end
end

// stdlib
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// isotree
#include <isotree.hpp>

// rice
#include <rice/rice.hpp>

using Rice::Array;
using Rice::Hash;
using Rice::Object;
using Rice::String;
using Rice::Symbol;

namespace Rice::detail {
  template<>
  class From_Ruby<NewCategAction> {
  public:
    NewCategAction convert(VALUE x) {
      auto value = Object(x).to_s().str();
      if (value == "weighted" || value == "impute") return Weighted;
      if (value == "smallest") return Smallest;
      if (value == "random") return Random;
      throw std::runtime_error("Unknown new categ action: " + value);
    }
  };

  template<>
  class From_Ruby<MissingAction> {
  public:
    MissingAction convert(VALUE x) {
      auto value = Object(x).to_s().str();
      if (value == "divide") return Divide;
      if (value == "impute") return Impute;
      if (value == "fail") return Fail;
      throw std::runtime_error("Unknown missing action: " + value);
    }
  };

  template<>
  class From_Ruby<CategSplit> {
  public:
    CategSplit convert(VALUE x) {
      auto value = Object(x).to_s().str();
      if (value == "subset") return SubSet;
      if (value == "single_categ") return SingleCateg;
      throw std::runtime_error("Unknown categ split: " + value);
    }
  };

  template<>
  class From_Ruby<CoefType> {
  public:
    CoefType convert(VALUE x) {
      auto value = Object(x).to_s().str();
      if (value == "uniform") return Uniform;
      if (value == "normal") return Normal;
      throw std::runtime_error("Unknown coef type: " + value);
    }
  };

  template<>
  class From_Ruby<UseDepthImp> {
  public:
    UseDepthImp convert(VALUE x) {
      auto value = Object(x).to_s().str();
      if (value == "lower") return Lower;
      if (value == "higher") return Higher;
      if (value == "same") return Same;
      throw std::runtime_error("Unknown depth imp: " + value);
    }
  };

  template<>
  class From_Ruby<WeighImpRows> {
  public:
    WeighImpRows convert(VALUE x) {
      auto value = Object(x).to_s().str();
      if (value == "inverse") return Inverse;
      if (value == "prop") return Prop;
      if (value == "flat") return Flat;
      throw std::runtime_error("Unknown weight imp rows: " + value);
    }
  };

  template<>
  class From_Ruby<ScoringMetric> {
  public:
    ScoringMetric convert(VALUE x) {
      auto value = Object(x).to_s().str();
      if (value == "depth") return Depth;
      if (value == "adj_depth") return AdjDepth;
      if (value == "density") return Density;
      if (value == "adj_density") return AdjDensity;
      if (value == "boxed_density") return BoxedDensity;
      if (value == "boxed_density2") return BoxedDensity2;
      if (value == "boxed_ratio") return BoxedRatio;
      throw std::runtime_error("Unknown scoring metric: " + value);
    }
  };
} // namespace Rice::detail

extern "C"
void Init_ext() {
  Rice::Module rb_mIsoTree = Rice::define_module("IsoTree");

  Rice::Module rb_mExt = Rice::define_module_under(rb_mIsoTree, "Ext");
  Rice::define_class_under<ExtIsoForest>(rb_mExt, "ExtIsoForest");

  rb_mExt
    .define_singleton_function(
      "fit_iforest",
      [](Hash options) {
        // model
        ExtIsoForest iso;

        // data
        size_t nrows = options.get<size_t, Symbol>("nrows");
        size_t ncols_numeric = options.get<size_t, Symbol>("ncols_numeric");
        size_t ncols_categ = options.get<size_t, Symbol>("ncols_categ");

        real_t* numeric_data = NULL;
        if (ncols_numeric > 0) {
          numeric_data = (double*) options.get<String, Symbol>("numeric_data").c_str();
        }

        int* categorical_data = NULL;
        int* ncat = NULL;
        if (ncols_categ > 0) {
          categorical_data = (int*) options.get<String, Symbol>("categorical_data").c_str();
          ncat = (int*) options.get<String, Symbol>("ncat").c_str();
        }

        // not used (sparse matrices)
        real_t* Xc = NULL;
        sparse_ix* Xc_ind = NULL;
        sparse_ix* Xc_indptr = NULL;

        // options
        // Rice has limit of 14 arguments, so use hash
        size_t sample_size = options.get<size_t, Symbol>("sample_size");
        size_t ndim = options.get<size_t, Symbol>("ndim");
        size_t ntrees = options.get<size_t, Symbol>("ntrees");
        size_t ntry = options.get<size_t, Symbol>("ntry");
        double prob_pick_by_gain_avg = options.get<double, Symbol>("prob_pick_avg_gain");
        double prob_pick_by_gain_pl = options.get<double, Symbol>("prob_pick_pooled_gain");
        double min_gain = options.get<double, Symbol>("min_gain");
        MissingAction missing_action = options.get<MissingAction, Symbol>("missing_action");
        CategSplit cat_split_type = options.get<CategSplit, Symbol>("categ_split_type");
        NewCategAction new_cat_action = options.get<NewCategAction, Symbol>("new_categ_action");
        bool all_perm = options.get<bool, Symbol>("all_perm");
        bool coef_by_prop = options.get<bool, Symbol>("coef_by_prop");
        bool with_replacement = options.get<bool, Symbol>("sample_with_replacement");
        bool penalize_range = options.get<bool, Symbol>("penalize_range");
        bool weigh_by_kurt = options.get<bool, Symbol>("weigh_by_kurtosis");
        CoefType coef_type = options.get<CoefType, Symbol>("coefs");
        size_t min_imp_obs = options.get<size_t, Symbol>("min_imp_obs");
        UseDepthImp depth_imp = options.get<UseDepthImp, Symbol>("depth_imp");
        WeighImpRows weigh_imp_rows = options.get<WeighImpRows, Symbol>("weigh_imp_rows");
        uint64_t random_seed = options.get<uint64_t, Symbol>("random_seed");
        bool use_long_double = options.get<bool, Symbol>("use_long_double");
        int nthreads = options.get<int, Symbol>("nthreads");

        // TODO options
        double* sample_weights = NULL;
        bool weight_as_sample = options.get<bool, Symbol>("weights_as_sample_prob");
        size_t max_depth = options.get<size_t, Symbol>("max_depth");
        bool limit_depth = options.get<bool, Symbol>("limit_depth");
        bool standardize_dist = false;
        double* tmat = NULL;
        double* output_depths = NULL;
        bool standardize_depth = false;
        real_t* col_weights = NULL;
        Imputer* imputer = NULL;
        bool impute_at_fit = false;

        int ncols_per_tree = options.get<int, Symbol>("ncols_per_tree");
        bool standardize_data = options.get<bool, Symbol>("standardize_data");
        ScoringMetric scoring_metric = options.get<ScoringMetric, Symbol>("scoring_metric");
        bool fast_bratio = options.get<bool, Symbol>("fast_bratio");
        double prob_pick_by_full_gain = options.get<double, Symbol>("prob_pick_full_gain");
        double prob_pick_by_dens = options.get<double, Symbol>("prob_pick_dens");
        double prob_pick_col_by_range = options.get<double, Symbol>("prob_pick_col_by_range");
        double prob_pick_col_by_var = options.get<double, Symbol>("prob_pick_col_by_var");
        double prob_pick_col_by_kurt = options.get<double, Symbol>("prob_pick_col_by_kurt");

        fit_iforest(
          NULL,
          &iso,
          numeric_data,
          ncols_numeric,
          categorical_data,
          ncols_categ,
          ncat,
          Xc,
          Xc_ind,
          Xc_indptr,
          ndim,
          ntry,
          coef_type,
          coef_by_prop,
          sample_weights,
          with_replacement,
          weight_as_sample,
          nrows,
          sample_size,
          ntrees,
          max_depth,
          ncols_per_tree,
          limit_depth,
          penalize_range,
          standardize_data,
          scoring_metric,
          fast_bratio,
          standardize_dist,
          tmat,
          output_depths,
          standardize_depth,
          col_weights,
          weigh_by_kurt,
          prob_pick_by_gain_pl,
          prob_pick_by_gain_avg,
          prob_pick_by_full_gain,
          prob_pick_by_dens,
          prob_pick_col_by_range,
          prob_pick_col_by_var,
          prob_pick_col_by_kurt,
          min_gain,
          missing_action,
          cat_split_type,
          new_cat_action,
          all_perm,
          imputer,
          min_imp_obs,
          depth_imp,
          weigh_imp_rows,
          impute_at_fit,
          random_seed,
          use_long_double,
          nthreads
        );

        return iso;
      })
    .define_singleton_function(
      "predict_iforest",
      [](ExtIsoForest& iso, Hash options) {
        // data
        size_t nrows = options.get<size_t, Symbol>("nrows");
        size_t ncols_numeric = options.get<size_t, Symbol>("ncols_numeric");
        size_t ncols_categ = options.get<size_t, Symbol>("ncols_categ");

        real_t* numeric_data = NULL;
        if (ncols_numeric > 0) {
          numeric_data = (double*) options.get<String, Symbol>("numeric_data").c_str();
        }

        int* categorical_data = NULL;
        if (ncols_categ > 0) {
          categorical_data = (int*) options.get<String, Symbol>("categorical_data").c_str();
        }

        // not used (sparse matrices)
        real_t* Xc = NULL;
        sparse_ix* Xc_ind = NULL;
        sparse_ix* Xc_indptr = NULL;
        real_t* Xr = NULL;
        sparse_ix* Xr_ind = NULL;
        sparse_ix* Xr_indptr = NULL;

        // options
        int nthreads = options.get<int, Symbol>("nthreads");
        bool standardize = options.get<bool, Symbol>("standardize");
        std::vector<double> outlier_scores(nrows);
        sparse_ix* tree_num = NULL;
        bool is_col_major = true;
        size_t ld_numeric = 0;
        size_t ld_categ = 0;
        double* per_tree_depths = NULL;

        predict_iforest(
          numeric_data,
          categorical_data,
          is_col_major,
          ld_numeric,
          ld_categ,
          Xc,
          Xc_ind,
          Xc_indptr,
          Xr,
          Xr_ind,
          Xr_indptr,
          nrows,
          nthreads,
          standardize,
          NULL,
          &iso,
          outlier_scores.data(),
          tree_num,
          per_tree_depths,
          NULL
        );

        Array ret;
        for (size_t i = 0; i < outlier_scores.size(); i++) {
          ret.push(outlier_scores[i], false);
        }
        return ret;
      })
    .define_singleton_function(
      "serialize_combined",
      [](ExtIsoForest& iso, String path, String metadata) {
        #ifdef _MSC_VER
        // TODO convert to wchar_t
        throw std::runtime_error("Not supported on Windows yet");
        #else
        std::ofstream file;
        file.open(path.c_str());
        serialize_combined(
          NULL,
          &iso,
          NULL,
          NULL,
          metadata.c_str(),
          // returns bytesize (RSTRING_LEN)
          metadata.length(),
          file
        );
        file.close();
        #endif
      })
    .define_singleton_function(
      "deserialize_combined",
      [](String path) {
        #ifdef _MSC_VER
        // TODO convert to wchar_t
        throw std::runtime_error("Not supported on Windows yet");
        #else
        Array ret;

        std::ifstream file;
        file.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
        if (!file) {
          throw std::runtime_error("Cannot open file");
        }

        bool is_isotree_model = false;
        bool is_compatible = false;
        bool has_combined_objects = false;
        bool has_IsoForest = false;
        bool has_ExtIsoForest = false;
        bool has_Imputer = false;
        bool has_Indexer = false;
        bool has_metadata = false;
        size_t size_metadata = 0;

        inspect_serialized_object(
          file,
          is_isotree_model,
          is_compatible,
          has_combined_objects,
          has_IsoForest,
          has_ExtIsoForest,
          has_Imputer,
          has_Indexer,
          has_metadata,
          size_metadata
        );

        if (!is_isotree_model || !has_combined_objects) {
          throw std::runtime_error("Input file is not a serialized isotree model");
        }
        if (!is_compatible) {
          throw std::runtime_error("Model file format is incompatible");
        }
        if (size_metadata == 0) {
          throw std::runtime_error("Input file does not contain metadata");
        }

        IsoForest model = IsoForest();
        ExtIsoForest model_ext = ExtIsoForest();
        Imputer imputer = Imputer();
        TreesIndexer indexer = TreesIndexer();
        char *optional_metadata = (char*) calloc(size_metadata, sizeof(char));
        if (optional_metadata == NULL) {
          throw std::runtime_error("Cannot allocate memory");
        }

        deserialize_combined(file, &model, &model_ext, &imputer, &indexer, optional_metadata);
        file.close();

        ret.push(Object(Rice::detail::To_Ruby<ExtIsoForest>().convert(model_ext)), false);
        ret.push(String(std::string(optional_metadata, size_metadata)), false);

        free(optional_metadata);

        return ret;
        #endif
      });
}

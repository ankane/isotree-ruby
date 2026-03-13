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
        auto nrows = options.get<size_t, Symbol>("nrows");
        auto ncols_numeric = options.get<size_t, Symbol>("ncols_numeric");
        auto ncols_categ = options.get<size_t, Symbol>("ncols_categ");

        real_t* numeric_data = nullptr;
        if (ncols_numeric > 0) {
          numeric_data = (double*) options.get<String, Symbol>("numeric_data").c_str();
        }

        int* categorical_data = nullptr;
        int* ncat = nullptr;
        if (ncols_categ > 0) {
          categorical_data = (int*) options.get<String, Symbol>("categorical_data").c_str();
          ncat = (int*) options.get<String, Symbol>("ncat").c_str();
        }

        // not used (sparse matrices)
        real_t* Xc = nullptr;
        sparse_ix* Xc_ind = nullptr;
        sparse_ix* Xc_indptr = nullptr;

        // options
        // Rice has limit of 14 arguments, so use hash
        auto sample_size = options.get<size_t, Symbol>("sample_size");
        auto ndim = options.get<size_t, Symbol>("ndim");
        auto ntrees = options.get<size_t, Symbol>("ntrees");
        auto ntry = options.get<size_t, Symbol>("ntry");
        auto prob_pick_by_gain_avg = options.get<double, Symbol>("prob_pick_avg_gain");
        auto prob_pick_by_gain_pl = options.get<double, Symbol>("prob_pick_pooled_gain");
        auto min_gain = options.get<double, Symbol>("min_gain");
        auto missing_action = options.get<MissingAction, Symbol>("missing_action");
        auto cat_split_type = options.get<CategSplit, Symbol>("categ_split_type");
        auto new_cat_action = options.get<NewCategAction, Symbol>("new_categ_action");
        auto all_perm = options.get<bool, Symbol>("all_perm");
        auto coef_by_prop = options.get<bool, Symbol>("coef_by_prop");
        auto with_replacement = options.get<bool, Symbol>("sample_with_replacement");
        auto penalize_range = options.get<bool, Symbol>("penalize_range");
        auto weigh_by_kurt = options.get<bool, Symbol>("weigh_by_kurtosis");
        auto coef_type = options.get<CoefType, Symbol>("coefs");
        auto min_imp_obs = options.get<size_t, Symbol>("min_imp_obs");
        auto depth_imp = options.get<UseDepthImp, Symbol>("depth_imp");
        auto weigh_imp_rows = options.get<WeighImpRows, Symbol>("weigh_imp_rows");
        auto random_seed = options.get<uint64_t, Symbol>("random_seed");
        auto use_long_double = options.get<bool, Symbol>("use_long_double");
        auto nthreads = options.get<int, Symbol>("nthreads");

        // TODO options
        double* sample_weights = nullptr;
        auto weight_as_sample = options.get<bool, Symbol>("weights_as_sample_prob");
        auto max_depth = options.get<size_t, Symbol>("max_depth");
        auto limit_depth = options.get<bool, Symbol>("limit_depth");
        bool standardize_dist = false;
        double* tmat = nullptr;
        double* output_depths = nullptr;
        bool standardize_depth = false;
        real_t* col_weights = nullptr;
        Imputer* imputer = nullptr;
        bool impute_at_fit = false;

        auto ncols_per_tree = options.get<int, Symbol>("ncols_per_tree");
        auto standardize_data = options.get<bool, Symbol>("standardize_data");
        auto scoring_metric = options.get<ScoringMetric, Symbol>("scoring_metric");
        auto fast_bratio = options.get<bool, Symbol>("fast_bratio");
        auto prob_pick_by_full_gain = options.get<double, Symbol>("prob_pick_full_gain");
        auto prob_pick_by_dens = options.get<double, Symbol>("prob_pick_dens");
        auto prob_pick_col_by_range = options.get<double, Symbol>("prob_pick_col_by_range");
        auto prob_pick_col_by_var = options.get<double, Symbol>("prob_pick_col_by_var");
        auto prob_pick_col_by_kurt = options.get<double, Symbol>("prob_pick_col_by_kurt");

        fit_iforest(
          nullptr,
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
        auto nrows = options.get<size_t, Symbol>("nrows");
        auto ncols_numeric = options.get<size_t, Symbol>("ncols_numeric");
        auto ncols_categ = options.get<size_t, Symbol>("ncols_categ");

        real_t* numeric_data = nullptr;
        if (ncols_numeric > 0) {
          numeric_data = (double*) options.get<String, Symbol>("numeric_data").c_str();
        }

        int* categorical_data = nullptr;
        if (ncols_categ > 0) {
          categorical_data = (int*) options.get<String, Symbol>("categorical_data").c_str();
        }

        // not used (sparse matrices)
        real_t* Xc = nullptr;
        sparse_ix* Xc_ind = nullptr;
        sparse_ix* Xc_indptr = nullptr;
        real_t* Xr = nullptr;
        sparse_ix* Xr_ind = nullptr;
        sparse_ix* Xr_indptr = nullptr;

        // options
        auto nthreads = options.get<int, Symbol>("nthreads");
        auto standardize = options.get<bool, Symbol>("standardize");
        std::vector<double> outlier_scores(nrows);
        sparse_ix* tree_num = nullptr;
        bool is_col_major = true;
        size_t ld_numeric = 0;
        size_t ld_categ = 0;
        double* per_tree_depths = nullptr;

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
          nullptr,
          &iso,
          outlier_scores.data(),
          tree_num,
          per_tree_depths,
          nullptr
        );

        Array ret;
        for (auto v : outlier_scores) {
          ret.push(v, false);
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
          nullptr,
          &iso,
          nullptr,
          nullptr,
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
        char* optional_metadata = static_cast<char*>(calloc(size_metadata, sizeof(char)));
        if (optional_metadata == nullptr) {
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

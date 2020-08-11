// isotree
#include <isotree.hpp>

// rice
#include <rice/Array.hpp>
#include <rice/Hash.hpp>
#include <rice/Module.hpp>
#include <rice/String.hpp>
#include <rice/Symbol.hpp>

using Rice::Array;
using Rice::Hash;
using Rice::Module;
using Rice::String;
using Rice::Symbol;
using Rice::define_class_under;
using Rice::define_module;

extern "C"
void Init_ext()
{
  Module rb_mIsoTree = define_module("IsoTree");

  Module rb_mExt = define_module_under(rb_mIsoTree, "Ext");
  define_class_under<ExtIsoForest>(rb_mExt, "ExtIsoForest");

  rb_mExt
    .define_singleton_method(
      "fit_iforest",
      *[](Hash options) {
        // model
        ExtIsoForest iso;

        // data
        size_t nrows = options.get<size_t, Symbol>("nrows");
        size_t ncols = options.get<size_t, Symbol>("ncols");
        double* numeric_data = (double*) options.get<String, Symbol>("numeric_data").c_str();
        size_t ncols_numeric = ncols;
        int* categ_data = NULL;
        size_t ncols_categ = 0;
        int* ncat = NULL;
        double* Xc = NULL;
        sparse_ix* Xc_ind = NULL;
        sparse_ix* Xc_indptr = NULL;

        // options
        CoefType coef_type = Normal;
        double* sample_weights = NULL;
        bool weight_as_sample = false;
        size_t max_depth = 0;
        bool limit_depth = true;
        bool standardize_dist = false;
        double* tmat = NULL;
        double* output_depths = NULL;
        bool standardize_depth = false;
        double* col_weights = NULL;
        MissingAction missing_action = Impute;
        CategSplit cat_split_type = SubSet;
        NewCategAction new_cat_action = Smallest;
        Imputer *imputer = NULL;
        UseDepthImp depth_imp = Higher;
        WeighImpRows weigh_imp_rows = Inverse;
        bool impute_at_fit = false;

        // Rice has limit of 14 arguments, so use hash for options
        size_t sample_size = options.get<size_t, Symbol>("sample_size");
        size_t ndim = options.get<size_t, Symbol>("ndim");
        size_t ntrees = options.get<size_t, Symbol>("ntrees");
        size_t ntry = options.get<size_t, Symbol>("ntry");
        double prob_pick_by_gain_avg = options.get<double, Symbol>("prob_pick_avg_gain");
        double prob_split_by_gain_avg = options.get<double, Symbol>("prob_split_avg_gain");
        double prob_pick_by_gain_pl = options.get<double, Symbol>("prob_pick_pooled_gain");
        double prob_split_by_gain_pl = options.get<double, Symbol>("prob_split_pooled_gain");
        double min_gain = options.get<double, Symbol>("min_gain");
        bool all_perm = options.get<bool, Symbol>("all_perm");
        bool coef_by_prop = options.get<bool, Symbol>("coef_by_prop");
        bool with_replacement = options.get<bool, Symbol>("sample_with_replacement");
        bool penalize_range = options.get<bool, Symbol>("penalize_range");
        bool weigh_by_kurt = options.get<bool, Symbol>("weigh_by_kurtosis");
        size_t min_imp_obs = options.get<size_t, Symbol>("min_imp_obs");
        uint64_t random_seed = options.get<uint64_t, Symbol>("random_seed");
        int nthreads = options.get<int, Symbol>("nthreads");

        fit_iforest(
          NULL,
          &iso,
          numeric_data,
          ncols_numeric,
          categ_data,
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
          limit_depth,
          penalize_range,
          standardize_dist,
          tmat,
          output_depths,
          standardize_depth,
          col_weights,
          weigh_by_kurt,
          prob_pick_by_gain_avg,
          prob_split_by_gain_avg,
          prob_pick_by_gain_pl,
          prob_split_by_gain_pl,
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
          nthreads
        );

        return iso;
      })
    .define_singleton_method(
      "predict_iforest",
      *[](ExtIsoForest& iso, Hash options) {
        // data
        size_t nrows = options.get<size_t, Symbol>("nrows");
        double* numeric_data = (double*) options.get<String, Symbol>("numeric_data").c_str();
        int* categ_data = NULL;
        double* Xc = NULL;
        sparse_ix* Xc_ind = NULL;
        sparse_ix* Xc_indptr = NULL;
        double* Xr = NULL;
        sparse_ix* Xr_ind = NULL;
        sparse_ix* Xr_indptr = NULL;

        // options
        int nthreads = options.get<int, Symbol>("nthreads");
        bool standardize = true;
        std::vector<double> outlier_scores(nrows);
        sparse_ix* tree_num = NULL;

        predict_iforest(
          numeric_data,
          categ_data,
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
          tree_num
        );

        Array ret;
        for (size_t i = 0; i < outlier_scores.size(); i++) {
          ret.push(outlier_scores[i]);
        }
        return ret;
      });
}

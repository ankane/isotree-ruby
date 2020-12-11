// isotree
#include <isotree.hpp>

// rice
#include <rice/Array.hpp>
#include <rice/Hash.hpp>
#include <rice/Module.hpp>
#include <rice/Object.hpp>
#include <rice/String.hpp>
#include <rice/Symbol.hpp>

using Rice::Array;
using Rice::Hash;
using Rice::Module;
using Rice::Object;
using Rice::String;
using Rice::Symbol;
using Rice::define_class_under;
using Rice::define_module;

template<>
NewCategAction from_ruby<NewCategAction>(Object x)
{
  auto value = x.to_s().str();
  if (value == "weighted") return Weighted;
  if (value == "smallest") return Smallest;
  if (value == "random") return Random;
  throw std::runtime_error("Unknown new categ action: " + value);
}

template<>
MissingAction from_ruby<MissingAction>(Object x)
{
  auto value = x.to_s().str();
  if (value == "divide") return Divide;
  if (value == "impute") return Impute;
  if (value == "fail") return Fail;
  throw std::runtime_error("Unknown missing action: " + value);
}

template<>
CategSplit from_ruby<CategSplit>(Object x)
{
  auto value = x.to_s().str();
  if (value == "subset") return SubSet;
  if (value == "single_categ") return SingleCateg;
  throw std::runtime_error("Unknown categ split: " + value);
}

template<>
CoefType from_ruby<CoefType>(Object x)
{
  auto value = x.to_s().str();
  if (value == "uniform") return Uniform;
  if (value == "normal") return Normal;
  throw std::runtime_error("Unknown coef type: " + value);
}

template<>
UseDepthImp from_ruby<UseDepthImp>(Object x)
{
  auto value = x.to_s().str();
  if (value == "lower") return Lower;
  if (value == "higher") return Higher;
  if (value == "same") return Same;
  throw std::runtime_error("Unknown depth imp: " + value);
}

template<>
WeighImpRows from_ruby<WeighImpRows>(Object x)
{
  auto value = x.to_s().str();
  if (value == "inverse") return Inverse;
  if (value == "prop") return Prop;
  if (value == "flat") return Flat;
  throw std::runtime_error("Unknown weight imp rows: " + value);
}

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
        size_t ncols_numeric = options.get<size_t, Symbol>("ncols_numeric");
        size_t ncols_categ = options.get<size_t, Symbol>("ncols_categ");

        double *restrict numeric_data = NULL;
        if (ncols_numeric > 0) {
          numeric_data = (double*) options.get<String, Symbol>("numeric_data").c_str();
        }

        int *restrict categorical_data = NULL;
        int *restrict ncat = NULL;
        if (ncols_categ > 0) {
          categorical_data = (int*) options.get<String, Symbol>("categorical_data").c_str();
          ncat = (int*) options.get<String, Symbol>("ncat").c_str();
        }

        // not used (sparse matrices)
        double* Xc = NULL;
        sparse_ix* Xc_ind = NULL;
        sparse_ix* Xc_indptr = NULL;

        // options
        // Rice has limit of 14 arguments, so use hash
        size_t sample_size = options.get<size_t, Symbol>("sample_size");
        size_t ndim = options.get<size_t, Symbol>("ndim");
        size_t ntrees = options.get<size_t, Symbol>("ntrees");
        size_t ntry = options.get<size_t, Symbol>("ntry");
        double prob_pick_by_gain_avg = options.get<double, Symbol>("prob_pick_avg_gain");
        double prob_split_by_gain_avg = options.get<double, Symbol>("prob_split_avg_gain");
        double prob_pick_by_gain_pl = options.get<double, Symbol>("prob_pick_pooled_gain");
        double prob_split_by_gain_pl = options.get<double, Symbol>("prob_split_pooled_gain");
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
        int nthreads = options.get<int, Symbol>("nthreads");

        // TODO options
        double* sample_weights = NULL;
        bool weight_as_sample = false;
        size_t max_depth = 0;
        bool limit_depth = true;
        bool standardize_dist = false;
        double* tmat = NULL;
        double* output_depths = NULL;
        bool standardize_depth = false;
        double* col_weights = NULL;
        Imputer *imputer = NULL;
        bool impute_at_fit = false;
        bool handle_interrupt = false;

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
          handle_interrupt,
          nthreads
        );

        return iso;
      })
    .define_singleton_method(
      "predict_iforest",
      *[](ExtIsoForest& iso, Hash options) {
        // data
        size_t nrows = options.get<size_t, Symbol>("nrows");
        size_t ncols_numeric = options.get<size_t, Symbol>("ncols_numeric");
        size_t ncols_categ = options.get<size_t, Symbol>("ncols_categ");

        double *restrict numeric_data = NULL;
        if (ncols_numeric > 0) {
          numeric_data = (double*) options.get<String, Symbol>("numeric_data").c_str();
        }

        int *restrict categorical_data = NULL;
        if (ncols_categ > 0) {
          categorical_data = (int*) options.get<String, Symbol>("categorical_data").c_str();
        }

        // not used (sparse matrices)
        double* Xc = NULL;
        sparse_ix* Xc_ind = NULL;
        sparse_ix* Xc_indptr = NULL;
        double* Xr = NULL;
        sparse_ix* Xr_ind = NULL;
        sparse_ix* Xr_indptr = NULL;

        // options
        int nthreads = options.get<int, Symbol>("nthreads");
        bool standardize = options.get<bool, Symbol>("standardize");
        std::vector<double> outlier_scores(nrows);
        sparse_ix* tree_num = NULL;

        predict_iforest(
          numeric_data,
          categorical_data,
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

require_relative "test_helper"

class IsolationForestTest < Minitest::Test
  def test_hashes
    data = test_data
    model = IsoTree::IsolationForest.new(ntrees: 10, ndim: 3, nthreads: 1)
    model.fit(data)
    predictions = model.predict(data)
    expected = [0.4816470280716818, 0.46655713161582574, 0.5363011880474468]
    assert_elements_in_delta expected, predictions.first(3)
    assert_equal 100, predictions.each_with_index.max[1]
  end

  def test_array
    data = numeric_data
    model = IsoTree::IsolationForest.new(ntrees: 10, ndim: 3, nthreads: 1)
    model.fit(data)
    predictions = model.predict(data)
    expected = [0.454691875234909, 0.42805783155356797, 0.5460616479701705]
    assert_elements_in_delta expected, predictions.first(3)
    assert_equal 100, predictions.each_with_index.max[1]
  end

  def test_export
    skip "Not supported yet" if windows?

    data = test_data
    model = IsoTree::IsolationForest.new(ntrees: 10, ndim: 3, nthreads: 1)
    model.fit(data)
    original_predictions = model.predict(data)

    tempfile = Tempfile.new
    model.export_model(tempfile.path)
    model = IsoTree::IsolationForest.import_model(tempfile.path)
    predictions = model.predict(data)
    assert_elements_in_delta original_predictions, predictions
  end

  def test_import_to_python
    skip if !ENV["TEST_PYTHON"] || windows?

    model = IsoTree::IsolationForest.new(ntrees: 10, ndim: 3, nthreads: 1)
    model.fit(test_data)
    model.export_model("/tmp/model.bin", add_metada_file: true)
    assert_match "Name: 100", %x[python3 test/support/import.py]

    expected = JSON.parse(File.read("test/support/model.bin.metadata"))
    metadata = JSON.parse(File.read("/tmp/model.bin.metadata"))
    metadata["data_info"].reject! { |k, _| ["sym_numeric", "sym_categ"].include?(k) }
    assert_equal expected, metadata
  end

  def test_import_from_python
    skip "Not supported yet" if windows?

    model = IsoTree::IsolationForest.import_model("test/support/model.bin")
    predictions = model.predict(test_data.map { |v| v.transform_keys(&:to_s) })
    assert_equal 100, predictions.each_with_index.max[1]
  end

  def test_import_missing_file
    error = assert_raises do
      IsoTree::IsolationForest.import_model("missing.bin")
    end
    assert_equal "Cannot open file", error.message
  end

  def test_numo
    data = Numo::DFloat.cast(numeric_data)
    model = IsoTree::IsolationForest.new(ntrees: 10, ndim: 3, nthreads: 1)
    model.fit(data)
    predictions = model.predict(data)
    expected = [0.454691875234909, 0.42805783155356797, 0.5460616479701705]
    assert_elements_in_delta expected, predictions.first(3)
    assert_equal 100, predictions.each_with_index.max[1]
  end

  def test_rover
    require "rover"

    data = Rover::DataFrame.new(test_data)
    model = IsoTree::IsolationForest.new(ntrees: 10, ndim: 3, nthreads: 1)
    model.fit(data)
    predictions = model.predict(data)
    expected = [0.4816470280716818, 0.46655713161582574, 0.5363011880474468]
    assert_elements_in_delta expected, predictions.first(3)
    assert_equal 100, predictions.each_with_index.max[1]
  end

  def test_predict_output_avg_depth
    data = test_data
    model = IsoTree::IsolationForest.new(ntrees: 10, ndim: 3, nthreads: 1)
    model.fit(data)
    predictions = model.predict(data, output: "avg_depth")
    # different results on different platforms with same seed
    expected = [8.847458736905825, 9.23295785483866, 7.545738407619213]
    assert_elements_in_delta expected, predictions.first(3)
    assert_equal 100, predictions.each_with_index.min[1]
  end

  def test_not_fit
    model = IsoTree::IsolationForest.new
    error = assert_raises do
      model.predict([])
    end
    assert_equal "Not fit", error.message
  end

  def test_different_columns
    x = Numo::DFloat.new(101, 2).rand_norm
    model = IsoTree::IsolationForest.new
    model.fit(x)
    error = assert_raises(ArgumentError) do
      model.predict(Numo::DFloat.new(2, 101).rand_norm)
    end
    assert_equal "Input must have 2 columns for this model", error.message
  end

  def test_no_data
    model = IsoTree::IsolationForest.new
    error = assert_raises(ArgumentError) do
      model.fit([])
    end
    assert_equal "No data", error.message
  end

  def test_bad_size
    model = IsoTree::IsolationForest.new
    error = assert_raises(ArgumentError) do
      model.fit([[1, 2], [3]])
    end
    assert_equal "All rows must have the same number of columns", error.message
  end

  def test_bad_dimensions
    model = IsoTree::IsolationForest.new
    error = assert_raises(ArgumentError) do
      model.fit(Numo::DFloat.cast([[[1]]]))
    end
    assert_equal "Input must have 2 dimensions", error.message
  end

  def test_bad_sample_size
    data = test_data
    model = IsoTree::IsolationForest.new(ntrees: 10, ndim: 3, nthreads: 1, sample_size: data.size * 2)
    model.fit(data)
  end

  def test_data
    CSV.table("test/support/data.csv").map(&:to_h)
  end

  def numeric_data
    test_data.map { |v| [v[:num1], v[:num2]] }
  end

  def windows?
    Gem.win_platform?
  end
end

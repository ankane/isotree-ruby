module IsoTree
  class Dataset
    attr_reader :numeric_columns, :categorical_columns, :array_type

    def initialize(data)
      @data = data

      if defined?(Rover::DataFrame) && data.is_a?(Rover::DataFrame)
        @vectors = data.vectors
        @numeric_columns, @categorical_columns = data.keys.partition { |k, v| ![:object, :bool].include?(data[k].type) }
        @array_type = false
      elsif defined?(Numo::NArray) && data.is_a?(Numo::NArray)
        raise ArgumentError, "Input must have 2 dimensions" if data.ndim != 2

        data = data.cast_to(Numo::DFloat)
        ncols = data.shape[1]

        @numeric_columns = ncols.times.to_a
        @categorical_columns = []

        @vectors = {}
        @numeric_columns.each do |k|
          @vectors[k] = data[true, k]
        end
        @array_type = true
      else
        data = data.to_a

        hashes = data.all? { |d| d.is_a?(Hash) }
        arrays = !hashes && data.all? { |d| d.is_a?(Array) }
        unless hashes || arrays
          raise ArgumentError, "Array elements must be all hashes or arrays"
        end

        ncols = data.first ? data.first.size : 0
        if data.any? { |r| r.size != ncols }
          raise ArgumentError, "All rows must have the same number of columns"
        end

        keys =
          if hashes
            data.flat_map(&:keys).uniq
          else
            ncols.times.to_a
          end

        @vectors = {}
        keys.each do |k|
          @vectors[k] = []
        end
        data.each do |d|
          keys.each do |k|
            @vectors[k] << d[k]
          end
        end

        @numeric_columns, @categorical_columns = keys.partition { |k| @vectors[k].all? { |v| v.nil? || v.is_a?(Numeric) } }
        @array_type = arrays
      end

      raise ArgumentError, "No data" if size == 0
    end

    def [](k)
      @vectors[k]
    end

    def size
      @vectors.any? ? @vectors.values.first.size : 0
    end
  end
end

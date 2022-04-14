//===----------------------------------------------------------------------===//
//                         DuckDB
//
// parquet_reader.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb.hpp"
#ifndef DUCKDB_AMALGAMATION
#include "duckdb/common/common.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/types/data_chunk.hpp"
#endif
#include "column_reader.hpp"
#include "parquet_file_metadata_cache.hpp"
#include "parquet_rle_bp_decoder.hpp"
#include "parquet_types.h"
#include "resizable_buffer.hpp"

#include <exception>

namespace duckdb_parquet {
namespace format {
class FileMetaData;
}
} // namespace duckdb_parquet

namespace duckdb {
class Allocator;
class ClientContext;
class ChunkCollection;
class BaseStatistics;
class TableFilterSet;

// TODO: Types of prefetching:
// 1. Small Row Groups: (num_threads * COMPRESSED_ROW_GROUP_SIZE < MAX_BUFFER_SIZE)
// 		a) No filters: we just prefetch the whole row_group
//		b) With filters: do some heurstics to determine if we want to still do it.
// 2. Large Row Groups: (AVG(total_compressed_size) per column > 1MB)
//		a) No filters: prefetch columns we will scan
//		b) With filters: do some heurstics to determine if we want to still do it.
struct ParquetReaderPrefetchConfig {
	/// The upper limit below which whole row groups will be prefetched
	static constexpr size_t WHOLE_GROUP_PREFETCH_LIMIT = 1 << 23; // 8 MiB
	/// Percentage of data in a row group that should be scanned for enabling whole group prefetch
	static constexpr double WHOLE_GROUP_PREFETCH_MINIMUM_SCAN = 0.75;

	/// The lower limit below which column chunks will not be cached
	static constexpr size_t COLUMN_CHUNK_CACHE_LIMIT = 1 << 20; // 1 MiB
};

struct ParquetReaderScanState {
	vector<idx_t> group_idx_list;
	int64_t current_group;
	vector<column_t> column_ids;
	idx_t group_offset;
	unique_ptr<FileHandle> file_handle;
	unique_ptr<ColumnReader> root_reader;
	unique_ptr<duckdb_apache::thrift::protocol::TProtocol> thrift_file_proto;

	bool finished;
	TableFilterSet *filters;
	SelectionVector sel;

	ResizeableBuffer define_buf;
	ResizeableBuffer repeat_buf;

	bool have_prefetched_group = false;
	idx_t prefetched_group;
};

struct ParquetOptions {
	explicit ParquetOptions() {
	}
	explicit ParquetOptions(ClientContext &context);

	bool binary_as_string = false;
};

class ParquetReader {
public:
	ParquetReader(Allocator &allocator, unique_ptr<FileHandle> file_handle_p,
	              const vector<LogicalType> &expected_types_p, const string &initial_filename_p = string());
	ParquetReader(Allocator &allocator, unique_ptr<FileHandle> file_handle_p)
	    : ParquetReader(allocator, move(file_handle_p), vector<LogicalType>(), string()) {
	}

	ParquetReader(ClientContext &context, string file_name, const vector<string> &names,
	              const vector<LogicalType> &expected_types_p, const vector<column_t> &column_ids,
	              ParquetOptions parquet_options, const string &initial_filename = string());
	ParquetReader(ClientContext &context, string file_name, ParquetOptions parquet_options)
	    : ParquetReader(context, move(file_name), vector<string>(), vector<LogicalType>(), vector<column_t>(),
	                    parquet_options, string()) {
	}
	ParquetReader(ClientContext &context, string file_name, const vector<LogicalType> &expected_types_p,
	              ParquetOptions parquet_options)
	    : ParquetReader(context, move(file_name), vector<string>(), expected_types_p, vector<column_t>(),
	                    parquet_options, string()) {
	}
	~ParquetReader();

	Allocator &allocator;
	string file_name;
	FileOpener *file_opener;
	vector<LogicalType> return_types;
	vector<string> names;
	shared_ptr<ParquetFileMetadataCache> metadata;
	ParquetOptions parquet_options;

public:
	void InitializeScan(ParquetReaderScanState &state, vector<column_t> column_ids, vector<idx_t> groups_to_read,
	                    TableFilterSet *table_filters);
	void Scan(ParquetReaderScanState &state, DataChunk &output);

	idx_t NumRows();
	idx_t NumRowGroups();

	const duckdb_parquet::format::FileMetaData *GetFileMetadata();

	static unique_ptr<BaseStatistics> ReadStatistics(ParquetReader &reader, LogicalType &type, column_t column_index,
	                                                 const duckdb_parquet::format::FileMetaData *file_meta_data);
	static LogicalType DeriveLogicalType(const SchemaElement &s_ele, bool binary_as_string);

private:
	void InitializeSchema(const vector<string> &names, const vector<LogicalType> &expected_types_p,
	                      const vector<column_t> &column_ids, const string &initial_filename_p);
	bool ScanInternal(ParquetReaderScanState &state, DataChunk &output);
	unique_ptr<ColumnReader> CreateReader(const duckdb_parquet::format::FileMetaData *file_meta_data);

	unique_ptr<ColumnReader> CreateReaderRecursive(const duckdb_parquet::format::FileMetaData *file_meta_data,
	                                               idx_t depth, idx_t max_define, idx_t max_repeat,
	                                               idx_t &next_schema_idx, idx_t &next_file_idx);
	const duckdb_parquet::format::RowGroup &GetGroup(ParquetReaderScanState &state);
	void PrepareRowGroupBuffer(ParquetReaderScanState &state, idx_t out_col_idx);
	LogicalType DeriveLogicalType(const SchemaElement &s_ele);

	template <typename... Args>
	std::runtime_error FormatException(const string fmt_str, Args... params) {
		return std::runtime_error("Failed to read Parquet file \"" + file_name +
		                          "\": " + StringUtil::Format(fmt_str, params...));
	}

private:
	unique_ptr<FileHandle> file_handle;
	//! column-id map, used when reading multiple parquet files since separate parquet files might have columns at
	//! different positions e.g. the first file might have column "a" at position 0, the second at position 1, etc
	vector<column_t> column_id_map;
	//! Map of column_id -> cast, used when reading multiple parquet files when parquet files have diverging types
	//! for the same column
	unordered_map<column_t, LogicalType> cast_map;
};

} // namespace duckdb

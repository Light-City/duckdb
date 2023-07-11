//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/format_serializer.hpp"
#include "duckdb/common/serializer/format_deserializer.hpp"
#include "duckdb/parser/parsed_data/create_info.hpp"
#include "duckdb/parser/parsed_data/create_index_info.hpp"

namespace duckdb {

void CreateInfo::FormatSerialize(FormatSerializer &serializer) const {
	serializer.WriteProperty("type", type);
	serializer.WriteProperty("catalog", catalog);
	serializer.WriteProperty("schema", schema);
	serializer.WriteProperty("temporary", temporary);
	serializer.WriteProperty("internal", internal);
	serializer.WriteProperty("on_conflict", on_conflict);
	serializer.WriteProperty("sql", sql);
}

unique_ptr<CreateInfo> CreateInfo::FormatDeserialize(FormatDeserializer &deserializer) {
	auto type = deserializer.ReadProperty<CatalogType>("type");
	auto catalog = deserializer.ReadProperty<string>("catalog");
	auto schema = deserializer.ReadProperty<string>("schema");
	auto temporary = deserializer.ReadProperty<bool>("temporary");
	auto internal = deserializer.ReadProperty<bool>("internal");
	auto on_conflict = deserializer.ReadProperty<OnCreateConflict>("on_conflict");
	auto sql = deserializer.ReadProperty<string>("sql");
	unique_ptr<CreateInfo> result;
	switch (type) {
	case CatalogType::INDEX_ENTRY:
		result = CreateIndexInfo::FormatDeserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of CreateInfo!");
	}
	result->catalog = std::move(catalog);
	result->schema = std::move(schema);
	result->temporary = temporary;
	result->internal = internal;
	result->on_conflict = on_conflict;
	result->sql = std::move(sql);
	return result;
}

void CreateIndexInfo::FormatSerialize(FormatSerializer &serializer) const {
	CreateInfo::FormatSerialize(serializer);
	serializer.WriteProperty("name", index_name);
	serializer.WriteProperty("index_type", index_type);
	serializer.WriteProperty("constraint_type", constraint_type);
	serializer.WriteProperty("parsed_expressions", parsed_expressions);
	serializer.WriteProperty("names", names);
	serializer.WriteProperty("column_ids", column_ids);
}

unique_ptr<CreateInfo> CreateIndexInfo::FormatDeserialize(FormatDeserializer &deserializer) {
	auto result = duckdb::unique_ptr<CreateIndexInfo>(new CreateIndexInfo());
	deserializer.ReadProperty("name", result->index_name);
	deserializer.ReadProperty("index_type", result->index_type);
	deserializer.ReadProperty("constraint_type", result->constraint_type);
	deserializer.ReadProperty("parsed_expressions", result->parsed_expressions);
	deserializer.ReadProperty("names", result->names);
	deserializer.ReadProperty("column_ids", result->column_ids);
	return std::move(result);
}

} // namespace duckdb

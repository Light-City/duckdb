#include "duckdb/planner/operator/logical_empty_result.hpp"

namespace duckdb {

LogicalEmptyResult::LogicalEmptyResult(unique_ptr<LogicalOperator> op)
    : LogicalOperator(LogicalOperatorType::LOGICAL_EMPTY_RESULT) {

	this->bindings = op->GetColumnBindings();

	op->ResolveOperatorTypes();
	this->return_types = op->types;
}

void LogicalEmptyResult::Serialize(FieldWriter &writer) const {
	throw NotImplementedException(LogicalOperatorToString(type));
}

unique_ptr<LogicalOperator> LogicalEmptyResult::Deserialize(ClientContext &context, LogicalOperatorType type,
                                                            FieldReader &reader) {
	throw NotImplementedException(LogicalOperatorToString(type));
}

} // namespace duckdb

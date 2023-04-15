#include "execution/filter_executer.hpp"
#include "pure.hpp"
#include "expression/comparsion_expression.hpp"
#include "storage/table_heap/table_heap.hpp"
#include "execution/seqscan_executer.hpp"
#include "expression/filter_expression.hpp"
#include "execution/filter_executer.hpp"
#include "expression/constant_expression.hpp"
#include "expression/column_value_expression.hpp"

PURE_TEST_INIT();

Schema* table_schema() {
  std::vector<Column> col;
  col.push_back(Column{ColumnType::INT, "id", 0});
  col.push_back(Column{ColumnType::INT, "age", 0});
  col.push_back(Column{ColumnType::VARCHAR, "name", 0});
  Schema* res = new Schema{col};
  return res;
}

Schema* out_schema(Schema* from, std::vector<size_t> idxs)
{
  Schema *res = new Schema{{}};
  res->copy_schema_from(from, idxs);
  return res;
}

void basic_test() {
  TableHeap tp{"test.db"};

  auto schema = table_schema();
  // age and name
  auto projection_schema = out_schema(schema, {1,2});
  auto tuple1 = Tuple::create_tuple(init_values(0,20,"pink1"), schema);
  auto tuple2 = Tuple::create_tuple(init_values(1,21,"pink2"), schema);
  auto tuple3 = Tuple::create_tuple(init_values(2,20,"pink3"), schema);
  tp.insert(Record{tuple1.data_.begin(), tuple1.data_.end()});
  tp.insert(Record{tuple2.data_.begin(), tuple2.data_.end()});
  tp.insert(Record{tuple3.data_.begin(), tuple3.data_.end()});

  SeqScanExectuer *seq_scan = new SeqScanExectuer{tp.scanner(), schema};

  std::vector<std::unique_ptr<Expression>> child_expr;

  Value v;
  v = 1;
  ConstantExpression* cs = new ConstantExpression{v};
  ColumnValueExpression* cv = new ColumnValueExpression{0};
  ComparsionExpression* cmp = new ComparsionExpression{cv, cs, ComparsionType::GREATER_THAN};

  child_expr.emplace_back(cmp);

  FilterExpression* express = new FilterExpression{std::move(child_expr)};
  FilterExecuter* filter = new FilterExecuter{express, seq_scan};

  Tuple * t = new Tuple{{}};
  filter->next(t);
  auto should_be_2 = t->value_at(schema, 0).get<int>();
  pure_assert(should_be_2 == 2) << should_be_2;

  remove("test.db");
}


int main(int , char**) {
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(basic_test);
  PURE_TEST_RUN();
}
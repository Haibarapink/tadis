#include "pure.hpp"
#include "execution/projection_executer.hpp"
#include "storage/table_heap/table_heap.hpp"
#include "expression/column_value_expression.hpp"
#include "expression/comparsion_expression.hpp"
#include "expression/projection_expression.hpp"
#include "execution/seqscan_executer.hpp"

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
  ProjecetionExecuter project{};

  std::vector<std::unique_ptr<Expression>> child_expr;

  // 1
  child_expr.emplace_back(new ColumnValueExpression{1});
  // 2
  child_expr.emplace_back(new ColumnValueExpression{2});

  ProjectionExpression *expr= new ProjectionExpression{std::move(child_expr)};

  project.schema_ = std::unique_ptr<Schema>{out_schema(schema, {1,2})};
  project.expression_ = std::unique_ptr<ProjectionExpression>{expr};
  project.children_.push_back(std::unique_ptr<Executer>{seq_scan});

  Tuple * t = new Tuple{{}};
  RC  rc = project.next(t);
  std::cout << t->to_string(schema);

  remove("test.db");
}

int main(int , char**) {
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(basic_test);
  PURE_TEST_RUN();
}
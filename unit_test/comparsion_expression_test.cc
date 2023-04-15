#include "expression/comparsion_expression.hpp"
#include "expression/constant_expression.hpp"
#include "expression/column_value_expression.hpp"

#include "pure.hpp"
PURE_TEST_INIT();

template <typename T>
void value_cmp_value_test(T left, T right, ComparsionType type , int should_be) {
  Value l_v, r_v;
  l_v = left;
  r_v = right;
  ConstantExpression* l = new ConstantExpression{l_v};
  ConstantExpression * r =new ConstantExpression{r_v};
  ComparsionExpression eq{l, r, type};

  auto compare_res = eq.evaluate(nullptr, nullptr);
  auto res = compare_res.get<int>();
  pure_assert(res == should_be) << res << " left " << left <<  " right " << right;
}

//value compare value
void vpv_test() {
  value_cmp_value_test("hello", "hello", ComparsionType::EQUAL, 1);
  value_cmp_value_test(1,2,ComparsionType::EQUAL, -1);
  value_cmp_value_test(1,2,ComparsionType::LESS_THAN, 1);
  value_cmp_value_test(2,1,ComparsionType::LESS_THAN, -1);
  value_cmp_value_test("fuck", "hello", ComparsionType::GREATER_THAN, -1);
  value_cmp_value_test("fuck", "hello", ComparsionType::LESS_THAN, 1);
}

void tuple_value_test() {
  auto values = init_values(1,2,3,"hello world");
  std::vector<Column> cols;
  for (auto i = 0; i < 3; ++i) {
    Column col{ColumnType::INT, std::string("int") + std::to_string(i), 0};
    cols.push_back(col);
  }

  Column col{ColumnType::VARCHAR, "name", 0};
  cols.push_back(col);

  Schema schema{std::move(cols)};
  Tuple t = Tuple::create_tuple(values, &schema);

  ColumnValueExpression *col_value_expr__idx_2 = new ColumnValueExpression{2};
  ColumnValueExpression * col_value_expr_idx_3 = new ColumnValueExpression{3};

  Value ok_name, err_name;
  ok_name = "hello world";
  err_name = "fuck world";

//  std::cout << t.value_at(&schema, 3).get<std::string>() << std::endl;

  ConstantExpression *ok_expr = new ConstantExpression{ok_name};
  ConstantExpression *err_expr = new ConstantExpression{err_name};

  ComparsionExpression expr{col_value_expr_idx_3, ok_expr, ComparsionType::EQUAL};
  auto cmp_val = expr.evaluate(&t, &schema);
  pure_assert(cmp_val.get<int>() == 1) << cmp_val.get<int>();

  ColumnValueExpression * col_value_expr_idx_33 = new ColumnValueExpression{3};
  ComparsionExpression expr2{col_value_expr_idx_33, err_expr, ComparsionType::EQUAL};
  auto cmp_val2 = expr2.evaluate(&t, &schema);
  pure_assert(cmp_val2.get<int>() == -1) << cmp_val.get<int>();
}


int main(int, char**) {
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(vpv_test);
  PURE_TEST_CASE(tuple_value_test);
  PURE_TEST_RUN();
}
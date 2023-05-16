#include "catalog/catalog.hpp"
#include "catalog/column.hpp"

int main(int, char**) {
    Catalog catalog{"."};
    std::vector<Column> cols;
    cols.emplace_back(ColumnType::CHAR, "asd", 100);
    catalog.create_table("fuck", cols);
}
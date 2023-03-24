/**
 * @brief Join operator that performs a join between two input tables based on a join predicate.
 * 
 * This operator uses a hash join algorithm to perform the join.
 * 
 * @tparam JoinPredicate The type of join predicate used to perform the join.
 */
template <typename JoinPredicate>
class JoinOperator : public Operator {
public:
    /**
     * @brief Construct a new Join Operator object
     * 
     * @param left_input The left input table to join.
     * @param right_input The right input table to join.
     * @param join_predicate The join predicate used to perform the join.
     */
    JoinOperator(std::shared_ptr<Operator> left_input, std::shared_ptr<Operator> right_input, JoinPredicate join_predicate)
        : left_input_(left_input), right_input_(right_input), join_predicate_(join_predicate) {}

    /**
     * @brief Get the next tuple from the join result.
     * 
     * @return std::optional<Tuple> The next tuple from the join result, or std::nullopt if there are no more tuples.
     */
    std::optional<Tuple> next() override {
        // TODO: Implement hash join 
        return std::nullopt;
    }

private:
    std::shared_ptr<Operator> left_input_;
    std::shared_ptr<Operator> right_input_;
    JoinPredicate join_predicate_;
};

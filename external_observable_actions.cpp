

// Action to listen to from the associated token contract to ensure registering should be allowed.
/**
 *
 * @param from The account to observe as the source of funds for a transfer
 * @param to The account to observe as the destination of funds for a transfer
 * @param quantity
 * @param memo A string to attach to a transaction. For staking this string should match the name of the running contract eg "dacelections". Otherwise it will be regarded only as a generic transfer to the account.
 * This action is intended only to observe transfers that are run by the associated token contract for the purpose of tracking the moving weights of votes if either the `from` or `to` in the transfer have active votes. It is not included in the ABI to prevent it from being called from outside the chain.
 */
void daccustodian::transfer(name from,
                            name to,
                            asset quantity,
                            string memo) {
    eosio::print("\nlistening to transfer with memo == dacaccountId");
    if (to == _self) {
        account_name dacId = eosio::string_to_name(memo.c_str());
        if (is_account(dacId)) {
            pendingstake_table_t pendingstake(_self, dacId);
            auto source = pendingstake.find(from);
            if (source != pendingstake.end()) {
                pendingstake.modify(source, _self, [&](tempstake &s) {
                    s.quantity += quantity;
                });
            } else {
                pendingstake.emplace(_self, [&](tempstake &s) {
                    s.sender = from;
                    s.quantity = quantity;
                    s.memo = memo;
                });
            }
        }
    }

    eosio::print("\n > transfer from : ", from, " to: ", to, " quantity: ", quantity);

    if (quantity.symbol == configs().lockupasset.symbol) {
        // Update vote weight for the 'from' in the transfer if vote exists
        auto existingVote = votes_cast_by_members.find(from);
        if (existingVote != votes_cast_by_members.end()) {
            updateVoteWeights(existingVote->candidates, -quantity.amount);
            _currentState.total_weight_of_votes -= quantity.amount;
        }

        // Update vote weight for the 'to' in the transfer if vote exists
        existingVote = votes_cast_by_members.find(to);
        if (existingVote != votes_cast_by_members.end()) {
            updateVoteWeights(existingVote->candidates, quantity.amount);
            _currentState.total_weight_of_votes += quantity.amount;
        }
    }
}
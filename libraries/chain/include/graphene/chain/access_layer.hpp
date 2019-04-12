/*
 * MIT License
 *
 * Copyright (c) 2018 Tech Solutions Malta LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <graphene/chain/database.hpp>
#include <graphene/chain/frequency_history_record_object.hpp>
#include <graphene/chain/queue_objects.hpp>

#include <fc/optional.hpp>
#include <fc/string.hpp>

#include <vector>

namespace graphene {
namespace chain {

// Structs for serializing data:

struct acc_id_res {

    acc_id_res() = default;
    acc_id_res(account_id_type account_id)
        : account_id(account_id)
    {
    }

    account_id_type account_id;
};

struct cycle_agreement {

    cycle_agreement() = default;
    cycle_agreement(share_type cycles, frequency_type frequency_lock)
        : cycles(cycles)
        , frequency_lock(frequency_lock)
    {
    }

    share_type cycles = 0;
    frequency_type frequency_lock = 0;
};

struct total_cycles_res {

    total_cycles_res() = default;
    total_cycles_res(share_type total_cycles, share_type total_dascoin)
        : total_cycles(total_cycles)
        , total_dascoin(total_dascoin)
    {
    }

    share_type total_cycles = 0;
    share_type total_dascoin = 0;
};

struct cycles_res {
    cycles_res() = default;
    cycles_res(share_type cycles, share_type dascoin)
        : cycles(cycles)
        , dascoin(dascoin)
    {
    }

    cycles_res operator+ (cycles_res const &obj)
    {
        return cycles_res{cycles + obj.cycles, dascoin + obj.dascoin};
    }

    share_type cycles = 0;
    share_type dascoin = 0;
};

struct autosubmit_res {
    autosubmit_res() = default;
    autosubmit_res(cycles_res charter, cycles_res utility, cycles_res package, cycles_res after_all_upgrades)
        : charter(charter)
        , utility(utility)
        , package(package)
        , after_all_upgrades(after_all_upgrades)
    {
        this->total = charter + utility + package;
    }

    autosubmit_res operator+ (autosubmit_res const &obj)
    {
        return autosubmit_res{charter + obj.charter, utility + obj.utility, package + obj.package, after_all_upgrades + obj.after_all_upgrades};
    }

    cycles_res total;
    cycles_res charter;
    cycles_res utility;
    cycles_res package;
    cycles_res after_all_upgrades;
};

struct manual_submit_res {
    manual_submit_res() = default;
    manual_submit_res(cycles_res tethered, cycles_res untethered)
        : tethered(tethered)
        , untethered(untethered)
    {
    }

    manual_submit_res operator+ (manual_submit_res const &obj)
    {
        return manual_submit_res{tethered + obj.tethered, untethered + obj.untethered};
    }

    cycles_res tethered;
    cycles_res untethered;
};

struct queue_projection_res {
    queue_projection_res() = default;
    queue_projection_res(autosubmit_res auto_submit, manual_submit_res total_locked, manual_submit_res next_locked, manual_submit_res utility, manual_submit_res package, cycles_res after_all_upgrades)
        : auto_submit(auto_submit)
        , total_locked_manual_submit(total_locked)
        , next_upgrade_last_locked_manual_submit(next_locked)
        , utility_manual_submit(utility)
        , package_manual_submit(package)
        , after_all_upgrades_manual_submit(after_all_upgrades)
    {
    }

    queue_projection_res operator+ (queue_projection_res const &obj)
    {
        return queue_projection_res(auto_submit + obj.auto_submit, total_locked_manual_submit + obj.total_locked_manual_submit,
                                    next_upgrade_last_locked_manual_submit + obj.next_upgrade_last_locked_manual_submit,
                                    utility_manual_submit + obj.utility_manual_submit, package_manual_submit + obj.package_manual_submit,
                                    after_all_upgrades_manual_submit + obj.after_all_upgrades_manual_submit);
    }

    autosubmit_res auto_submit;
    manual_submit_res total_locked_manual_submit;
    manual_submit_res next_upgrade_last_locked_manual_submit;
    manual_submit_res utility_manual_submit;
    manual_submit_res package_manual_submit;
    cycles_res after_all_upgrades_manual_submit;
};

struct acc_id_share_t_res : public acc_id_res {

    using result_t = optional<share_type>;

    acc_id_share_t_res() = default;
    acc_id_share_t_res(account_id_type account_id, result_t result)
        : acc_id_res(account_id)
        , result(result)
    {
    }

    result_t result = 0;
};

struct acc_id_cycle_agreement_res : public acc_id_res {

    using result_t = optional<cycle_agreement>;

    acc_id_cycle_agreement_res() = default;
    acc_id_cycle_agreement_res(account_id_type account_id, result_t result = {})
        : acc_id_res(account_id)
        , result(result)
    {
    }

    result_t result;
};

struct acc_id_vec_cycle_agreement_res : public acc_id_res {

    using result_t = optional<vector<cycle_agreement>>;

    acc_id_vec_cycle_agreement_res() = default;
    acc_id_vec_cycle_agreement_res(account_id_type id, result_t result = {})
        : acc_id_res(id)
        , result(result)
    {
    }

    result_t result;
};

struct sub_w_pos {

    using pos_t = uint32_t;

    sub_w_pos() = default;
    sub_w_pos(pos_t position, reward_queue_object submission)
        : position(position)
        , submission(submission)
    {
    }

    pos_t position;
    reward_queue_object submission;
};

struct acc_id_queue_subs_w_pos_res : public acc_id_res {

    using result_t = optional<vector<sub_w_pos>>;

    acc_id_queue_subs_w_pos_res() = default;
    acc_id_queue_subs_w_pos_res(account_id_type account_id, result_t result = {})
        : acc_id_res(account_id)
        , result(result)
    {
    }

    result_t result;
};

struct vault_info_res {
    share_type cash_balance;
    share_type reserved_balance;
    share_type dascoin_balance;
    share_type free_cycle_balance;
    share_type dascoin_limit;
    share_type eur_limit;
    share_type spent;
    bool       is_tethered;
    uint32_t   owner_change_counter;
    uint32_t   active_change_counter;
    optional<license_information_object> license_information;

    vault_info_res() = default;
    explicit vault_info_res(share_type cash_balance,
        share_type reserved_balance,
        share_type dascoin_balance,
        share_type free_cycle_balance,
        share_type dascoin_limit,
        share_type eur_limit,
        share_type spent,
        bool is_tethered,
        uint32_t   owner_change_counter,
        uint32_t   active_change_counter,
        optional<license_information_object> license_information)
    : cash_balance(cash_balance),
      reserved_balance(reserved_balance),
      dascoin_balance(dascoin_balance),
      free_cycle_balance(free_cycle_balance),
      dascoin_limit(dascoin_limit),
      eur_limit(eur_limit),
      spent(spent),
      is_tethered(is_tethered),
      owner_change_counter(owner_change_counter),
      active_change_counter(active_change_counter),
      license_information(license_information) {}
};

struct acc_id_vault_info_res : public acc_id_res {
    
    using result_t = optional<vault_info_res>;

    acc_id_vault_info_res() = default;
    explicit acc_id_vault_info_res(account_id_type account_id, result_t result = {})
        : acc_id_res(account_id), result(result) {}

    result_t result;
};

struct license_types_grouped_by_kind_res {
    struct license_name_and_id {
        string name;
        license_type_id_type id;
    };

    license_types_grouped_by_kind_res() = default;
    explicit license_types_grouped_by_kind_res(license_kind kind, vector<license_name_and_id> licenses)
      : kind(kind), licenses(move(licenses))
    {}
    license_kind kind;
    vector<license_name_and_id> licenses;
};

struct license_objects_grouped_by_kind_res {
    license_objects_grouped_by_kind_res() = default;
    explicit license_objects_grouped_by_kind_res(license_kind kind, vector<license_type_object> licenses)
      : kind(kind), licenses(move(licenses))
    {}
    license_kind kind;
    vector<license_type_object> licenses;
};

struct das33_project_tokens_amount {
  das33_project_tokens_amount() = default;
  explicit das33_project_tokens_amount(asset pledge, asset base, asset bonus)
    : pledge(pledge),base(base), bonus(bonus)
  {}
  asset pledge;
  asset base;
  asset bonus;
};

struct das33_pledges_by_account_result {
  das33_pledges_by_account_result() = default;
  map<das33_project_id_type, share_type> total_expected;
  map<das33_project_id_type, share_type> base_expected_in_last_round;
  vector<das33_pledge_holder_object> pledges;
};

struct signed_block_with_num
{
  uint32_t num;
  block_id_type block_id;
  signed_block block;

  signed_block_with_num() = default;
  explicit signed_block_with_num(uint32_t num, block_id_type block_id, signed_block block)
    : num(num), block_id(block_id), block(block) {}
};

struct signed_block_with_virtual_operations_and_num
{
  uint32_t num;
  block_id_type block_id;
  signed_block_with_virtual_operations block;

  signed_block_with_virtual_operations_and_num() = default;
  explicit signed_block_with_virtual_operations_and_num(uint32_t num, block_id_type block_id, signed_block_with_virtual_operations block)
    : num(num), block_id(block_id), block(block) {}
};

class database;
class global_property_object;
class reward_queue_object;

using fc::optional;
using fc::string;

class database_access_layer {
  public:
    explicit database_access_layer(const database& db)
        : _db(db){};
    ~database_access_layer() {}

    // Transactions and blocks:
    // TODO: expose get_block through this interface.
    vector<signed_block_with_num> get_blocks(uint32_t start_block_num, uint32_t count) const;
    vector<signed_block_with_virtual_operations_and_num> get_blocks_with_virtual_operations(uint32_t start_block_num,
                                                                                            uint32_t count,
                                                                                            std::vector<uint16_t>& virtual_operation_ids) const;
    // Global objects:
    global_property_object get_global_properties() const;

    // Balances:
    acc_id_share_t_res get_free_cycle_balance(account_id_type id) const;
    acc_id_vec_cycle_agreement_res get_all_cycle_balances(account_id_type id) const;
    acc_id_share_t_res get_dascoin_balance(account_id_type id) const;
    optional<total_cycles_res> get_total_cycles(account_id_type id) const;
    optional<queue_projection_res> get_queue_state_for_account(account_id_type id) const;

    vector<acc_id_share_t_res> get_free_cycle_balances_for_accounts(vector<account_id_type> ids) const;
    vector<acc_id_vec_cycle_agreement_res> get_all_cycle_balances_for_accounts(vector<account_id_type> ids) const;
    vector<acc_id_share_t_res> get_dascoin_balances_for_accounts(vector<account_id_type> ids) const;

    // License:
    vector<pair<string, license_type_id_type>> get_license_type_names_ids() const;
    vector<license_type_object> get_license_types() const;
    optional<license_type_object> get_license_type(string name) const;
    optional<license_type_object> get_license_type(license_type_id_type license_id) const;
    vector<license_types_grouped_by_kind_res> get_license_type_names_ids_grouped_by_kind() const;
    vector<license_objects_grouped_by_kind_res> get_license_objects_grouped_by_kind() const;

    // Queue:
    uint32_t get_reward_queue_size() const;
    vector<reward_queue_object> get_reward_queue() const;
    vector<reward_queue_object> get_reward_queue_by_page(uint32_t from, uint32_t amount) const;
    acc_id_queue_subs_w_pos_res get_queue_submissions_with_pos(account_id_type account_id) const;
    vector<acc_id_queue_subs_w_pos_res> get_queue_submissions_with_pos_for_accounts(vector<account_id_type> ids) const;

    // Requests:
    optional<issued_asset_record_object> get_issued_asset_record(const string& unique_id, asset_id_type asset_id) const;

    // Vaults:
    optional<vault_info_res> get_vault_info(account_id_type vault_id) const;
    vector<acc_id_vault_info_res> get_vaults_info(vector<account_id_type> vault_ids) const;

    // Assets:
    optional<asset_object> lookup_asset_symbol(const string& symbol_or_id) const;
    vector<optional<asset_object>> lookup_asset_symbols(const vector<string>& symbols_or_ids) const;
    bool check_issued_asset(const string& unique_id, const string& asset) const;
    bool check_issued_webeur(const string& unique_id) const;

    // Frequency:
    vector<frequency_history_record_object> get_frequency_history() const;
    vector<frequency_history_record_object> get_frequency_history_by_page(uint32_t from, uint32_t amount) const;

  private:
    optional<asset_object> get_asset_symbol(const asset_index &index, const string& symbol_or_id) const;

    template <typename IndexType>
    uint32_t size() const
    {
        return _db.get_index_type<IndexType>().indices().size();
    }

    template <typename QueryType, typename IndexType, typename IndexBy>
    typename IndexType::object_type get(QueryType id) const
    {
        const auto& idx = _db.get_index_type<IndexType>().indices().template get<IndexBy>();
        return *(idx.find(id));
    }

    template <typename QueryType, typename IndexType, typename IndexBy>
    optional<typename IndexType::object_type> get_opt(QueryType id) const
    {
        const auto& idx = _db.get_index_type<IndexType>().indices().template get<IndexBy>();
        auto it = idx.find(id);
        if (it != idx.end())
            return {*it};
        return {};
    }

    template <typename IndexType, typename IndexBy>
    vector<typename IndexType::object_type> get_all() const
    {
        const auto& idx = _db.get_index_type<IndexType>().indices().template get<IndexBy>();
        return vector<typename IndexType::object_type>(idx.begin(), idx.end());
    }

    template <typename IndexType, typename IndexBy, int MAX_ELEMENTS = 100>
    vector<typename IndexType::object_type> get_range(uint32_t from, uint32_t amount) const
    {
        const auto& idx = _db.get_index_type<IndexType>().indices().template get<IndexBy>();
        FC_ASSERT(idx.size() > from, "Index out of bounds, index: ${from}, size: ${size}", ("from", from)("size", idx.size()));
        FC_ASSERT(idx.size() - from >= amount, "Index out of bounds, amount: ${amount}, size: ${size}", ("amount", amount)("size", idx.size()));
        FC_ASSERT(amount <= MAX_ELEMENTS, "Cannot retrieve more than ${max} elements in one page", ("max", MAX_ELEMENTS));
        auto start = idx.begin();
        std::advance(start, from);
        auto end = idx.begin();
        std::advance(end, from + amount);
        return vector<typename IndexType::object_type>(start, end);
    }

    template <typename ReturnType>
    vector<ReturnType> get_balance(const vector<account_id_type>& ids, const std::function<ReturnType(account_id_type)>& getter) const
    {
        vector<ReturnType> result;
        result.reserve(ids.size());
        for (auto id : ids)
            result.emplace_back(getter(id));
        return result;
    }

    const database& _db;
};
}
}  // namespace graphene::chain

FC_REFLECT(graphene::chain::cycle_agreement, (cycles)(frequency_lock))
FC_REFLECT(graphene::chain::total_cycles_res, (total_cycles)(total_dascoin))
FC_REFLECT(graphene::chain::cycles_res, (cycles)(dascoin))
FC_REFLECT(graphene::chain::autosubmit_res, (total)(charter)(package)(utility)(after_all_upgrades))
FC_REFLECT(graphene::chain::manual_submit_res, (tethered)(untethered))
FC_REFLECT(graphene::chain::queue_projection_res, (auto_submit)(total_locked_manual_submit)(next_upgrade_last_locked_manual_submit)(utility_manual_submit)(package_manual_submit)(after_all_upgrades_manual_submit))

FC_REFLECT(graphene::chain::acc_id_res, (account_id))
FC_REFLECT_DERIVED(graphene::chain::acc_id_share_t_res, (graphene::chain::acc_id_res), (result))
FC_REFLECT_DERIVED(graphene::chain::acc_id_cycle_agreement_res, (graphene::chain::acc_id_res), (result))
FC_REFLECT_DERIVED(graphene::chain::acc_id_vec_cycle_agreement_res, (graphene::chain::acc_id_res), (result))

FC_REFLECT(graphene::chain::sub_w_pos, (position)(submission))
FC_REFLECT_DERIVED(graphene::chain::acc_id_queue_subs_w_pos_res, (graphene::chain::acc_id_res), (result))

FC_REFLECT( graphene::chain::signed_block_with_num, (num)(block_id)(block) )
FC_REFLECT( graphene::chain::signed_block_with_virtual_operations_and_num, (num)(block_id)(block) )

FC_REFLECT(graphene::chain::vault_info_res,
           (cash_balance)
           (reserved_balance)
           (dascoin_balance)
           (free_cycle_balance)
           (dascoin_limit)
           (eur_limit)
           (spent)
           (is_tethered)
           (owner_change_counter)
           (active_change_counter)
           (license_information))

FC_REFLECT_DERIVED(graphene::chain::acc_id_vault_info_res, (graphene::chain::acc_id_res), (result))

FC_REFLECT(graphene::chain::license_types_grouped_by_kind_res::license_name_and_id,
           (name)
           (id))

FC_REFLECT(graphene::chain::license_types_grouped_by_kind_res,
           (kind)
           (licenses))

FC_REFLECT(graphene::chain::license_objects_grouped_by_kind_res,
           (kind)
           (licenses))

FC_REFLECT(graphene::chain::das33_project_tokens_amount,
           (pledge)
           (base)
           (bonus))

FC_REFLECT(graphene::chain::das33_pledges_by_account_result,
           (pledges)
           (total_expected)
           (base_expected_in_last_round))

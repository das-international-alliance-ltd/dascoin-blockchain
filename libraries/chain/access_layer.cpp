#include <graphene/chain/access_layer.hpp>

#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/queue_objects.hpp>
#include <graphene/chain/issued_asset_record_object.hpp>

namespace graphene {
namespace chain {

global_property_object database_access_layer::get_global_properties() const
{
    return _db.get(global_property_id_type());
}

// Transactions and blocks:
vector<signed_block_with_num> database_access_layer::get_blocks(uint32_t start_block_num, uint32_t count) const
{
    FC_ASSERT(count > 0, "Must fetch at least one block");
    FC_ASSERT(count <= 100, "Too many blocks to fetch, limit is 100");
    auto head_block_num = _db.head_block_num();
    FC_ASSERT(start_block_num <= head_block_num,
              "Starting block ${start_n} is higher than current block height ${head_n}",
              ("start_n", start_block_num)
              ("head_n", head_block_num));

    vector<signed_block_with_num> result;
    result.reserve(count);
    auto end = start_block_num + count;
    if (end > head_block_num)
        end = head_block_num;
    for (auto i = start_block_num; i < end; ++i) {
        auto signed_block = _db.fetch_block_by_number(i);
        FC_ASSERT(signed_block.valid(),
                  "Block number ${num} could not be retreived",
                  ("num", i)
                 );
        const auto block_id = signed_block->id();
        result.emplace_back(i, block_id, *signed_block);
    }
    return result;
}

vector<signed_block_with_virtual_operations_and_num> database_access_layer::get_blocks_with_virtual_operations(uint32_t start_block_num,
                                                                                          uint32_t count,
                                                                                          std::vector<uint16_t>& virtual_operation_ids) const
{
    FC_ASSERT(count > 0, "Must fetch at least one block");
    FC_ASSERT(count <= 100, "Too many blocks to fetch, limit is 100");
    FC_ASSERT(start_block_num > 0, "Starting block must be higher than 0.");

    auto head_block_num = _db.head_block_num();
    FC_ASSERT(start_block_num <= head_block_num,
              "Starting block ${start_n} is higher than current block height ${head_n}",
              ("start_n", start_block_num)
              ("head_n", head_block_num));

    for(auto operation_id : virtual_operation_ids)
    {
          FC_ASSERT(operation_type_limits::is_virtual_operation(operation_id), "Operation id ${op_id} is not valid virtual operation id.",
                ("op_id", operation_id));
    }

    vector<signed_block_with_virtual_operations_and_num> result;
    result.reserve(count);
    auto end = start_block_num + count;
    if (end > head_block_num)
        end = head_block_num;
    for (auto i = start_block_num; i < end; ++i) {
        auto signed_block = _db.fetch_block_with_virtual_operations_by_number(i, virtual_operation_ids);
        FC_ASSERT(signed_block.valid(),
                  "Block number ${num} could not be retreived",
                  ("num", i)
                 );
        const auto block_id = signed_block->id();
        result.emplace_back(i, block_id, *signed_block);
    }
    return result;
}

// Balances:
acc_id_share_t_res database_access_layer::get_free_cycle_balance(account_id_type id) const
{
    auto cycle_balance_obj = get_opt<account_id_type, account_cycle_balance_index, by_account_id>(id);

    optional<share_type> opt_balance;
    if (cycle_balance_obj.valid())
        opt_balance = cycle_balance_obj->balance;

    return {id, opt_balance};
}

acc_id_share_t_res database_access_layer::get_dascoin_balance(account_id_type id) const
{
    auto key = boost::make_tuple(id, _db.get_dascoin_asset_id());
    auto balance_obj = get_opt<decltype(key), account_balance_index, by_account_asset>(key);

    optional<share_type> opt_balance;
    if (balance_obj.valid())
        opt_balance = balance_obj->balance;

    return {id, opt_balance};
}

acc_id_vec_cycle_agreement_res database_access_layer::get_all_cycle_balances(account_id_type id) const
{
    if (!get_opt<account_id_type, account_index, by_id>(id).valid())
        // TODO: ugly, figure out a way to use braces.
        return {id};  // Account with said id does not exist, return empty optional.

    vector<cycle_agreement> result;
    // First entry is for free cycle balances:
    auto cycle_balance_obj = get<account_id_type, account_cycle_balance_index, by_account_id>(id);
    result.emplace_back(cycle_balance_obj.balance, 0);

    // Rest of the entries are from the queue:
    const auto& queue_multi_idx = _db.get_index_type<reward_queue_index>().indices();
    const auto& account_idx = queue_multi_idx.get<by_account>();

    const auto& range = account_idx.equal_range(id);
    for (auto it = range.first; it != range.second; ++it)
        result.emplace_back(it->amount, it->frequency);

    return {id, {result}};
}

vector<acc_id_share_t_res>
    database_access_layer::get_free_cycle_balances_for_accounts(vector<account_id_type> ids) const
{
    return get_balance<acc_id_share_t_res>(ids, std::bind(&database_access_layer::get_free_cycle_balance,
                                                          this, std::placeholders::_1));
}

vector<acc_id_vec_cycle_agreement_res>
    database_access_layer::get_all_cycle_balances_for_accounts(vector<account_id_type> ids) const
{
    return get_balance<acc_id_vec_cycle_agreement_res>(ids, std::bind(&database_access_layer::get_all_cycle_balances,
                                                                       this, std::placeholders::_1));
}

vector<acc_id_share_t_res> database_access_layer::get_dascoin_balances_for_accounts(vector<account_id_type> ids) const
{
    return get_balance<acc_id_share_t_res>(ids, std::bind(&database_access_layer::get_dascoin_balance,
                                                          this, std::placeholders::_1));
}

optional<total_cycles_res> database_access_layer::get_total_cycles(account_id_type vault_id) const
{
    const auto& account = get_opt<account_id_type, account_index, by_id>(vault_id);
    if (account.valid() && account->is_vault())
    {
        auto license_information = _db.get_license_information(vault_id);
        if (license_information.valid() && license_information->is_manual_submit()) 
        {
            auto history = license_information->history;
            total_cycles_res result;
            for (auto itr = history.begin(); itr != history.end(); ++itr)
            {

                //TODO: Write helper function for code below:
                auto lic = get_license_type(itr->license);
                if (lic.valid())
                  if (!(lic->kind == license_kind::locked_frequency ||
                      lic->kind == license_kind::utility ||
                      lic->kind == license_kind::package))
                    continue;

                result.total_cycles += itr->amount;
                result.total_cycles += itr->non_upgradeable_amount;
                result.total_dascoin += _db.cycles_to_dascoin(itr->amount + itr->non_upgradeable_amount, itr->frequency_lock);
            }
            return result;
        }
    }
    return {};
}

optional<queue_projection_res> database_access_layer::get_queue_state_for_account(account_id_type id) const
{
    const auto& account = get_opt<account_id_type, account_index, by_id>(id);
    if (account.valid() && account->is_vault())
    {
        auto license_information = _db.get_license_information(id);
        if (license_information.valid())
        {
            auto history = license_information->history;
            queue_projection_res result;
            for (auto itr = history.begin(); itr != history.end(); ++itr)
            {
                auto lic = get_license_type(itr->license);
                if (lic.valid())
                {
                  cycles_res tmp;
                  if (lic->kind == license_kind::chartered)//  || (lic->kind == license_kind::locked_frequency && lic->up_policy == detail::president))
                  {
                      if (itr->balance_upgrade.used < itr->balance_upgrade.max)
                      {
                        tmp.cycles = itr->amount * itr->balance_upgrade.multipliers[itr->balance_upgrade.used];
                        tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                        result.auto_submit.charter = result.auto_submit.charter + tmp;
                        auto upgrades = itr->balance_upgrade.used;
                        tmp = cycles_res{0,0};
                        while(upgrades < itr->balance_upgrade.max)
                        {
                            tmp.cycles += itr->amount * itr->balance_upgrade.multipliers[upgrades];
                            upgrades++;
                        }
                        tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                        result.auto_submit.after_all_upgrades = result.auto_submit.after_all_upgrades + tmp;
                      }
                  }
                  else if (lic->kind == license_kind::locked_frequency)// && lic->up_policy != detail::president)
                  {
                      tmp.cycles = itr->amount;
                      tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);

                      cycles_res non_upgradeable{itr->non_upgradeable_amount, _db.cycles_to_dascoin(itr->non_upgradeable_amount, itr->frequency_lock)};

                      if (account->is_tethered())
                          result.total_locked_manual_submit.tethered = result.total_locked_manual_submit.tethered + tmp + non_upgradeable;
                      else
                          result.total_locked_manual_submit.untethered = result.total_locked_manual_submit.untethered + tmp + non_upgradeable;

                      if (itr->balance_upgrade.max - itr->balance_upgrade.used == 1)
                      {
                        if (lic->up_policy == detail::president)
                        {
                            tmp.cycles += 2 * (itr->base_amount + (itr->base_amount * itr->bonus_percent / 100));
                            tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                        }
                        else
                        {
                            tmp = tmp + tmp;
                        }

                        if (account->is_tethered())
                              result.next_upgrade_last_locked_manual_submit.tethered = result.next_upgrade_last_locked_manual_submit.tethered + tmp + non_upgradeable;
                          else
                              result.next_upgrade_last_locked_manual_submit.untethered = result.next_upgrade_last_locked_manual_submit.untethered + tmp + non_upgradeable;
                      }

                      if (itr->balance_upgrade.max > itr->balance_upgrade.used)
                      {
                          auto upgrades = itr->balance_upgrade.used;
                          auto amount = itr->amount;
                          while(upgrades < itr->balance_upgrade.max)
                          {
                              if (lic->up_policy != detail::president)
                                amount *= itr->balance_upgrade.multipliers[upgrades];
                              else
                                amount += (itr->base_amount + (itr->base_amount * itr->bonus_percent / 100)) * itr->balance_upgrade.multipliers[upgrades];
                              upgrades++;
                          }
                          tmp.cycles = amount;
                          tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                          result.after_all_upgrades_manual_submit = result.after_all_upgrades_manual_submit + tmp + non_upgradeable;
                      }
                      else
                      {
                          result.after_all_upgrades_manual_submit = result.after_all_upgrades_manual_submit + tmp + non_upgradeable;
                      }
                  }
                  else if (lic->kind == license_kind::utility)
                  {
                      if (itr->balance_upgrade.used < itr->balance_upgrade.max)
                      {
                          tmp.cycles = itr->base_amount * itr->balance_upgrade.multipliers[itr->balance_upgrade.used];
                          tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                          result.auto_submit.utility = result.auto_submit.utility + tmp;
                          auto upgrades = itr->balance_upgrade.used;
                          tmp.cycles = 0;
                          while (upgrades < itr->balance_upgrade.max)
                          {
                            tmp.cycles += itr->base_amount * itr->balance_upgrade.multipliers[upgrades];
                            upgrades++;
                          }
                          tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                          result.auto_submit.after_all_upgrades = result.auto_submit.after_all_upgrades + tmp;
                      }
                      tmp.cycles = itr->amount;
                      tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                      if (account->is_tethered())
                          result.utility_manual_submit.tethered = result.utility_manual_submit.tethered + tmp;
                      else
                          result.utility_manual_submit.untethered = result.utility_manual_submit.untethered + tmp;
                      result.after_all_upgrades_manual_submit = result.after_all_upgrades_manual_submit + tmp;
                  }
                  else if (lic->kind == license_kind::package)
                  {
                      if (itr->balance_upgrade.used < itr->balance_upgrade.max)
                      {
                          tmp.cycles = itr->base_amount * itr->balance_upgrade.multipliers[itr->balance_upgrade.used];
                          tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                          result.auto_submit.package = result.auto_submit.package + tmp;
                          auto upgrades = itr->balance_upgrade.used;
                          tmp.cycles = 0;
                          while (upgrades < itr->balance_upgrade.max)
                          {
                            tmp.cycles += itr->base_amount * itr->balance_upgrade.multipliers[upgrades];
                            upgrades++;
                          }
                          tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                          result.auto_submit.after_all_upgrades = result.auto_submit.after_all_upgrades + tmp;
                      }
                      tmp.cycles = itr->amount;
                      tmp.dascoin = _db.cycles_to_dascoin(tmp.cycles, itr->frequency_lock);
                      if (account->is_tethered())
                          result.package_manual_submit.tethered = result.package_manual_submit.tethered + tmp;
                      else
                          result.package_manual_submit.untethered = result.package_manual_submit.untethered + tmp;
                      result.after_all_upgrades_manual_submit = result.after_all_upgrades_manual_submit + tmp;
                  }
                }
            }
            return result;
        }
    }
    return {};
}

// License:
optional<license_type_object> database_access_layer::get_license_type(string name) const
{
    return get_opt<string, license_type_index, by_name>(name);
}

vector<license_type_object> database_access_layer::get_license_types() const
{
    return get_all<license_type_index, by_id>();
}

optional<license_type_object> database_access_layer::get_license_type(license_type_id_type license_id) const
{
  return get_opt<license_type_id_type, license_type_index, by_id>(license_id);
}

vector<pair<string, license_type_id_type>> database_access_layer::get_license_type_names_ids() const
{
    vector<pair<string, license_type_id_type>> result;
    for (const auto& lic : get_license_types())
        result.emplace_back(lic.name, lic.id);
    return result;
}

vector<license_types_grouped_by_kind_res> database_access_layer::get_license_type_names_ids_grouped_by_kind() const
{
    map<license_kind, vector<license_types_grouped_by_kind_res::license_name_and_id>> tmp;
    vector<license_types_grouped_by_kind_res> result;
    for (const auto& lic : get_license_types())
        tmp[lic.kind].emplace_back(license_types_grouped_by_kind_res::license_name_and_id{lic.name, lic.id});
    for (auto& lic : tmp)
        result.emplace_back(license_types_grouped_by_kind_res{lic.first, lic.second});
    return result;
}

vector<license_objects_grouped_by_kind_res> database_access_layer::get_license_objects_grouped_by_kind() const
{
    map<license_kind, vector<license_type_object>> tmp;
    vector<license_objects_grouped_by_kind_res> result;
    for (const auto& lic : get_license_types())
        tmp[lic.kind].emplace_back(lic);
    for (auto& lic : tmp)
        result.emplace_back(license_objects_grouped_by_kind_res{lic.first, lic.second});
    return result;
}

uint32_t database_access_layer::get_reward_queue_size() const { return size<reward_queue_index>(); }

vector<reward_queue_object> database_access_layer::get_reward_queue() const
{
    return get_all<reward_queue_index, by_time>();
}

vector<reward_queue_object> database_access_layer::get_reward_queue_by_page(uint32_t from, uint32_t amount) const
{
    return get_range<reward_queue_index, by_time>(from, amount);
}

vector<frequency_history_record_object> database_access_layer::get_frequency_history() const
{
    return get_all<frequency_history_record_index, by_time>();
}

vector<frequency_history_record_object> database_access_layer::get_frequency_history_by_page(uint32_t from, uint32_t amount) const
{
    return get_range<frequency_history_record_index, by_time>(from, amount);
}

acc_id_queue_subs_w_pos_res database_access_layer::get_queue_submissions_with_pos(account_id_type account_id) const
{
    if (!get_opt<account_id_type, account_index, by_id>(account_id).valid())
        return {account_id};  // Account does not exist, return null result.

    vector<sub_w_pos> result;

    const auto& queue_multi_idx = _db.get_index_type<reward_queue_index>().indices();
    const auto& account_idx = queue_multi_idx.get<by_account>();
    const auto& time_idx = queue_multi_idx.get<by_time>();

    const auto& range = account_idx.equal_range(account_id);
    for (auto it = range.first; it != range.second; ++it) {
        uint32_t pos = distance(time_idx.begin(), queue_multi_idx.project<by_time>(it));
        result.emplace_back(pos, *it);
    }

    return {account_id, {result}};
}

vector<acc_id_queue_subs_w_pos_res>
    database_access_layer::get_queue_submissions_with_pos_for_accounts(vector<account_id_type> ids) const
{
    return get_balance<acc_id_queue_subs_w_pos_res>(ids, std::bind(&database_access_layer::get_queue_submissions_with_pos,
                                                                   this, std::placeholders::_1));
}

optional<vault_info_res> database_access_layer::get_vault_info(account_id_type vault_id) const
{
    const auto& account = get_opt<account_id_type, account_index, by_id>(vault_id);

    // TODO: re-evaluate this, should we throw an error here?
    if (!account.valid() || !account->is_vault())
        return {};

    const auto& webeur_balance = _db.get_balance_object(vault_id, _db.get_web_asset_id());
    const auto& dascoin_balance = _db.get_balance_object(vault_id, _db.get_dascoin_asset_id());
    const auto& free_cycle_balance = _db.get_cycle_balance(vault_id);
    const auto& license_information = _db.get_license_information(vault_id);
    const auto& eur_limit = _db.get_eur_limit(license_information);

    return vault_info_res{webeur_balance.balance,
                          webeur_balance.reserved,
                          dascoin_balance.balance,
                          free_cycle_balance,
                          dascoin_balance.limit,
                          eur_limit,
                          dascoin_balance.spent,
                          account->is_tethered(),
                          account->owner_change_counter,
                          account->active_change_counter,
                          license_information};
}

vector<acc_id_vault_info_res> database_access_layer::get_vaults_info(vector<account_id_type> vault_ids) const
{
    return get_balance<acc_id_vault_info_res>(vault_ids, [this](account_id_type account_id) -> acc_id_vault_info_res {
        return acc_id_vault_info_res{account_id, this->get_vault_info(account_id)};
    });
}

optional<asset_object> database_access_layer::lookup_asset_symbol(const string& symbol_or_id) const
{
    return get_asset_symbol(_db.get_index_type<asset_index>(), symbol_or_id);
}

vector<optional<asset_object>> database_access_layer::lookup_asset_symbols(const vector<string>& symbols_or_ids) const
{
    const auto& assets_by_symbol = _db.get_index_type<asset_index>();
    vector<optional<asset_object>> result;
    result.reserve(symbols_or_ids.size());
    std::transform(symbols_or_ids.begin(), symbols_or_ids.end(), std::back_inserter(result),
                   std::bind(&database_access_layer::get_asset_symbol, this, std::ref(assets_by_symbol), std::placeholders::_1));

    return result;
}

bool database_access_layer::check_issued_asset(const string& unique_id, const string& asset) const
{
    const auto res = lookup_asset_symbol(asset);
    if ( res.valid() )
    {
        const auto record = get_issued_asset_record(unique_id, res->id);
        return record.valid();
    }
    return false;
}

bool database_access_layer::check_issued_webeur(const string& unique_id) const
{
    const auto web_id = _db.get_web_asset_id();
    return get_issued_asset_record(unique_id, web_id).valid();
}

optional<asset_object> database_access_layer::get_asset_symbol(const asset_index &index, const string& symbol_or_id) const
{
    const auto& asset_by_symbol = index.indices().get<by_symbol>();
    if( !symbol_or_id.empty() && std::isdigit(symbol_or_id[0]) )
    {
        auto ptr = _db.find(variant(symbol_or_id).as<asset_id_type>( 0 ));
        return ptr == nullptr ? optional<asset_object>{} : *ptr;
    }
    auto itr = asset_by_symbol.find(symbol_or_id);
    return itr == asset_by_symbol.end() ? optional<asset_object>{} : *itr;
}

// TODO:
optional<issued_asset_record_object>
database_access_layer::get_issued_asset_record(const string& unique_id, asset_id_type asset_id) const
{
    const auto& idx = _db.get_index_type<issued_asset_record_index>().indices().get<by_unique_id_asset>();
    auto it = idx.find(boost::make_tuple(unique_id, asset_id));
    if (it != idx.end())
        return {*it};
    return {};
}

}  // namespace chain
}  // namespace graphene

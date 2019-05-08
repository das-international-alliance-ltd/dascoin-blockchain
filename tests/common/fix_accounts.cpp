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

#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/protocol/asset_ops.hpp>
// #include <graphene/chain/fba_object.hpp>
// #include <graphene/chain/license_objects.hpp>
// #include <graphene/chain/market_object.hpp>
// #include <graphene/chain/vesting_balance_object.hpp>
// #include <graphene/chain/witness_object.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

using namespace graphene::chain::test;

namespace graphene { namespace chain {

const account_object& database_fixture::make_new_account_base(
   const account_kind kind,
   const account_id_type registrar,
   const string& name,
   const public_key_type& key /* = public_key_type() */)
{ try {
   account_create_operation op;

   op.kind = static_cast<uint8_t>(kind);
   op.name = name;
   op.registrar = registrar;
   op.owner = authority(123, key, 123);
   op.active = authority(321, key, 321);
   op.options.memo_key = key;
   op.options.voting_account = GRAPHENE_PROXY_TO_SELF_ACCOUNT;

   auto& active_committee_members = db.get_global_properties().active_committee_members;
   if( active_committee_members.size() > 0 )
   {
      set<vote_id_type> votes;
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      op.options.votes = flat_set<vote_id_type>(votes.begin(), votes.end());
   }
   op.options.num_committee = op.options.votes.size();

   op.fee = db.current_fee_schedule().calculate_fee( op );

   set_expiration( db, trx );
   trx.operations.clear();
   trx.operations.push_back(op);
   trx.validate();
   auto r = db.push_transaction(trx, ~0);
   generate_block();
   return db.get<account_object>(r.operation_results[0].get<object_id_type>());

} FC_CAPTURE_AND_RETHROW() }

  const account_object& database_fixture::create_new_account(
     const account_id_type registrar,
     const string& name,
     const public_key_type& key /* = public_key_type() */)
  { try {

     return make_new_account_base( account_kind::wallet, registrar, name, key );

  } FC_CAPTURE_AND_RETHROW() }

  const account_object& database_fixture::create_new_vault_account(
     const account_id_type registrar,
     const string& name,
     const public_key_type& key /* = public_key_type() */)
  { try {

     return make_new_account_base( account_kind::vault, registrar, name, key );

  } FC_CAPTURE_AND_RETHROW() }

  const account_object& database_fixture::create_new_custodian_account(
          const account_id_type registrar,
          const string& name,
          const public_key_type& key /* = public_key_type() */)
  { try {

      return make_new_account_base( account_kind::custodian, registrar, name, key );

    } FC_CAPTURE_AND_RETHROW() }


void database_fixture::tether_accounts(const account_id_type wallet, const account_id_type vault)
{ try {

  tether_accounts_operation op;
  op.wallet_account = wallet;
  op.vault_account = vault;

  set_expiration( db, trx );
  trx.operations.clear();
  trx.operations.push_back( op );
  trx.validate();
  processed_transaction ptx = db.push_transaction( trx, ~0 );

} FC_CAPTURE_AND_RETHROW ( (wallet)(vault) ) }

const account_balance_object& database_fixture::get_account_balance_object(account_id_type account_id, asset_id_type asset_id)
{
  return db.get_balance_object(account_id, asset_id);
}

void database_fixture::set_vault_to_wallet_limit_toggle(account_id_type account_id, bool flag)
{ try {
  const auto& account_obj = account_id(db);
  db.modify(account_obj, [flag](account_object& ao){
    ao.disable_vault_to_wallet_limit = flag;
  });
} FC_LOG_AND_RETHROW() }

void database_fixture::disable_vault_to_wallet_limit(account_id_type account_id)
{
  set_vault_to_wallet_limit_toggle(account_id, true);
}

void database_fixture::enable_vault_to_wallet_limit(account_id_type account_id)
{
  set_vault_to_wallet_limit_toggle(account_id, false);
}

void database_fixture::set_roll_back_enabled(account_id_type account_id, bool roll_back_enabled)
{
  set_roll_back_enabled_operation op;
  op.account = account_id;
  op.roll_back_enabled = roll_back_enabled;
  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();
}

void database_fixture::roll_back_public_keys(account_id_type authority, account_id_type account_id)
{
  roll_back_public_keys_operation op;
  op.authority = authority;
  op.account = account_id;
  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();
}

} }  // namespace graphene::chain

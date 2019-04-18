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

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/protocol/asset_ops.hpp>
#include <graphene/chain/protocol/wire.hpp>
#include <graphene/chain/wire_object.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

using namespace graphene::chain::test;

namespace graphene { namespace chain {

const wire_out_holder_object& database_fixture::wire_out(account_id_type account_id, asset asset_to_wire,
                                                         const string& memo)
{ try {

  wire_out_operation op;
  op.account = account_id;
  op.asset_to_wire = asset_to_wire;
  op.memo = memo;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();

  return db.get<wire_out_holder_object>(ptx.operation_results[0].get<object_id_type>());

} FC_LOG_AND_RETHROW() }

vector<wire_out_holder_object> database_fixture::get_wire_out_holders(account_id_type account_id,
                                                                      const flat_set<asset_id_type>& asset_ids) const
{
   vector<wire_out_holder_object> result;
   const auto& idx = db.get_index_type<wire_out_holder_index>().indices().get<by_account_asset>();
   if ( asset_ids.empty() )
   {
      // if the caller passes in an empty list of asset ID's, return holders for all assets the account owns
      for ( auto it = idx.find(boost::make_tuple(account_id)); it != std::end(idx) && it->account == account_id; ++it )
        result.emplace_back(*it);
   }
   else
   {
      for ( auto asset_id : asset_ids )
      {
        for ( auto it = idx.find(boost::make_tuple(account_id, asset_id));
              it != std::end(idx) && it->account == account_id;
              ++it
            )
          result.emplace_back(*it);
      }
   }
   return result;
}

void database_fixture::wire_out_complete(wire_out_holder_id_type holder_id)
{
  wire_out_complete_operation op;

  op.wire_out_handler = get_wire_out_handler_id();
  op.holder_object_id = holder_id;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();
}

void database_fixture::wire_out_reject(wire_out_holder_id_type holder_id)
{
  wire_out_reject_operation op;

  op.wire_out_handler = get_wire_out_handler_id();
  op.holder_object_id = holder_id;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();
}

} }  // namespace graphene::chain

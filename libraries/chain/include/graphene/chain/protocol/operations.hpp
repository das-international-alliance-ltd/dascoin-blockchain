/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/account.hpp>
#include <graphene/chain/protocol/assert.hpp>
#include <graphene/chain/protocol/asset_ops.hpp>
#include <graphene/chain/protocol/balance.hpp>
#include <graphene/chain/protocol/custom.hpp>
#include <graphene/chain/protocol/committee_member.hpp>
#include <graphene/chain/protocol/confidential.hpp>
#include <graphene/chain/protocol/cycle.hpp>
#include <graphene/chain/protocol/das33_operations.hpp>
#include <graphene/chain/protocol/daspay_operations.hpp>
#include <graphene/chain/protocol/fba.hpp>
#include <graphene/chain/protocol/fee_operations.hpp>
#include <graphene/chain/protocol/license.hpp>
#include <graphene/chain/protocol/market.hpp>
#include <graphene/chain/protocol/proposal.hpp>
#include <graphene/chain/protocol/transfer.hpp>
#include <graphene/chain/protocol/vesting.hpp>
#include <graphene/chain/protocol/upgrade.hpp>
#include <graphene/chain/protocol/update_globals.hpp>
#include <graphene/chain/protocol/wire.hpp>
#include <graphene/chain/protocol/wire_out_with_fee.hpp>
#include <graphene/chain/protocol/withdraw_permission.hpp>
#include <graphene/chain/protocol/witness.hpp>
#include <graphene/chain/protocol/worker.hpp>

namespace graphene { namespace chain {

   /**
    * @ingroup operations
    *
    * Defines the set of valid operations as a discriminated union type.
    */
   typedef fc::static_variant<
            transfer_operation,

            limit_order_create_operation,
            limit_order_cancel_operation,
            call_order_update_operation,

            account_create_operation,
            account_update_operation,
            account_whitelist_operation,
            account_upgrade_operation,
            account_transfer_operation,

            asset_create_operation,
            asset_update_operation,
            asset_update_bitasset_operation,
            asset_update_feed_producers_operation,
            asset_issue_operation,
            asset_reserve_operation,
            asset_fund_fee_pool_operation,
            asset_settle_operation,
            asset_global_settle_operation,
            asset_publish_feed_operation,

            witness_create_operation,
            witness_update_operation,

            proposal_create_operation,
            proposal_update_operation,
            proposal_delete_operation,

            withdraw_permission_create_operation,
            withdraw_permission_update_operation,
            withdraw_permission_claim_operation,
            withdraw_permission_delete_operation,

            committee_member_create_operation,
            committee_member_update_operation,
            committee_member_update_global_parameters_operation,

            vesting_balance_create_operation,
            vesting_balance_withdraw_operation,

            worker_create_operation,

            custom_operation,
            assert_operation,

            balance_claim_operation,

            override_transfer_operation,

            transfer_to_blind_operation,
            blind_transfer_operation,
            transfer_from_blind_operation,

            asset_claim_fees_operation,

            // Dascoin protocol operations:

            board_update_chain_authority_operation,
            update_queue_parameters_operation,

            create_license_type_operation,
            issue_license_operation,

            tether_accounts_operation,

            asset_create_issue_request_operation,
            asset_deny_issue_request_operation,

            wire_out_operation,
            wire_out_complete_operation,
            wire_out_reject_operation,

            transfer_vault_to_wallet_operation,
            transfer_wallet_to_vault_operation,

            submit_reserve_cycles_to_queue_operation,
            submit_cycles_to_queue_operation,

            change_public_keys_operation,

            update_global_frequency_operation,

            issue_free_cycles_operation,
            edit_license_type_operation,
            update_euro_limit_operation,

            submit_cycles_to_queue_by_license_operation,

            update_license_operation,
            issue_cycles_to_license_operation,

            remove_root_authority_operation,
            create_witness_operation,
            update_witness_operation,
            remove_witness_operation,
            activate_witness_operation,
            deactivate_witness_operation,

            create_upgrade_event_operation,
            update_upgrade_event_operation,
            delete_upgrade_event_operation,

            remove_vault_limit_operation,

            change_operation_fee_operation,
            change_fee_pool_account_operation,
            purchase_cycle_asset_operation,
            transfer_cycles_from_licence_to_wallet_operation,

            wire_out_with_fee_operation,
            wire_out_with_fee_complete_operation,
            wire_out_with_fee_reject_operation,

            set_starting_cycle_asset_amount_operation,

            set_roll_back_enabled_operation,
            roll_back_public_keys_operation,

            fee_pool_cycles_submit_operation,

            set_chain_authority_operation,
            register_daspay_authority_operation,
            unregister_daspay_authority_operation,
            set_daspay_transaction_ratio_operation,
            create_payment_service_provider_operation,
            update_payment_service_provider_operation,
            delete_payment_service_provider_operation,
            reserve_asset_on_account_operation,
            unreserve_asset_on_account_operation,
            daspay_debit_account_operation,
            daspay_credit_account_operation,
            update_daspay_clearing_parameters_operation,
            update_delayed_operations_resolver_parameters_operation,

            das33_project_create_operation,
            das33_project_update_operation,
            das33_project_delete_operation,
            das33_pledge_asset_operation,

            update_global_parameters_operation,
            update_external_btc_price_operation,

            das33_distribute_project_pledges_operation,
            das33_project_reject_operation,
            das33_distribute_pledge_operation,
            das33_pledge_reject_operation,
            das33_set_use_external_btc_price_operation,
            das33_set_use_market_price_for_token_operation,

            update_external_token_price_operation,
            daspay_set_use_external_token_price_operation,

            // Virtual operations below this point:

            record_submit_reserve_cycles_to_queue_operation,  // TODO: should we keep this op?
            record_submit_charter_license_cycles_operation,  // TODO: should we keep this op?

            record_distribute_dascoin_operation,

            asset_distribute_completed_request_operation,
            upgrade_account_cycles_operation,

            fba_distribute_operation,
            asset_settle_cancel_operation,
            fill_order_operation,
            wire_out_result_operation,
            wire_out_with_fee_result_operation,
            unreserve_completed_operation,
            das33_pledge_result_operation
   > operation;

   /// @} // operations group

   // this struct keeps index from which to which operation is regular operation
   // and from which to which operation is virtual operation
   // and needs to be updated accordingly when new operations are added
   struct operation_type_limits
   {
      static bool is_virtual_operation(const operation& op);
      static bool is_virtual_operation(const unsigned uop);
   private:
      static operation first_virtual_operation;
   };

   /**
    *  Appends required authorites to the result vector.  The authorities appended are not the
    *  same as those returned by get_required_auth
    *
    *  @return a set of required authorities for @ref op
    */
   void operation_get_required_authorities( const operation& op,
                                            flat_set<account_id_type>& active,
                                            flat_set<account_id_type>& owner,
                                            vector<authority>&  other );

   void operation_validate( const operation& op );

   /**
    *  @brief necessary to support nested operations inside the proposal_create_operation
    */
   struct op_wrapper
   {
      public:
         op_wrapper(const operation& op = operation()):op(op){}
         operation op;
   };

} } // graphene::chain

FC_REFLECT_TYPENAME( graphene::chain::operation )
FC_REFLECT( graphene::chain::op_wrapper, (op) )

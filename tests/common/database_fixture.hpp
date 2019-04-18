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

#include <graphene/app/application.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/access_layer.hpp>
#include <fc/io/json.hpp>

#include <graphene/chain/operation_history_object.hpp>

#include <iostream>

using namespace graphene::db;

extern uint32_t GRAPHENE_TESTING_GENESIS_TIMESTAMP;

#define PUSH_TX \
   graphene::chain::test::_push_transaction

#define PUSH_BLOCK \
   graphene::chain::test::_push_block

// See below
#define REQUIRE_OP_VALIDATION_SUCCESS( op, field, value ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   op.validate(); \
   op.field = temp; \
}
#define REQUIRE_OP_EVALUATION_SUCCESS( op, field, value ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   trx.operations.back() = op; \
   op.field = temp; \
   db.push_transaction( trx, ~0 ); \
}

#define GRAPHENE_REQUIRE_THROW( expr, exc_type )          \
{                                                         \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "GRAPHENE_REQUIRE_THROW begin "        \
         << req_throw_info << std::endl;                  \
   BOOST_REQUIRE_THROW( expr, exc_type );                 \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "GRAPHENE_REQUIRE_THROW end "          \
         << req_throw_info << std::endl;                  \
}

#define GRAPHENE_CHECK_THROW( expr, exc_type )            \
{                                                         \
   std::string req_throw_info = fc::json::to_string(      \
      fc::mutable_variant_object()                        \
      ("source_file", __FILE__)                           \
      ("source_lineno", __LINE__)                         \
      ("expr", #expr)                                     \
      ("exc_type", #exc_type)                             \
      );                                                  \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "GRAPHENE_CHECK_THROW begin "          \
         << req_throw_info << std::endl;                  \
   BOOST_CHECK_THROW( expr, exc_type );                   \
   if( fc::enable_record_assert_trip )                    \
      std::cout << "GRAPHENE_CHECK_THROW end "            \
         << req_throw_info << std::endl;                  \
}

#define REQUIRE_OP_VALIDATION_FAILURE_2( op, field, value, exc_type ) \
{ \
   const auto temp = op.field; \
   op.field = value; \
   GRAPHENE_REQUIRE_THROW( op.validate(), exc_type ); \
   op.field = temp; \
}
#define REQUIRE_OP_VALIDATION_FAILURE( op, field, value ) \
   REQUIRE_OP_VALIDATION_FAILURE_2( op, field, value, fc::exception )

#define REQUIRE_THROW_WITH_VALUE_2(op, field, value, exc_type) \
{ \
   auto bak = op.field; \
   op.field = value; \
   trx.operations.back() = op; \
   op.field = bak; \
   GRAPHENE_REQUIRE_THROW(db.push_transaction(trx, ~0), exc_type); \
}

#define REQUIRE_THROW_WITH_VALUE( op, field, value ) \
   REQUIRE_THROW_WITH_VALUE_2( op, field, value, fc::exception )

///This simply resets v back to its default-constructed value. Requires v to have a working assingment operator and
/// default constructor.
#define RESET(v) v = decltype(v)()
///This allows me to build consecutive test cases. It's pretty ugly, but it works well enough for unit tests.
/// i.e. This allows a test on update_account to begin with the database at the end state of create_account.
#define INVOKE(test) ((struct test*)this)->test_method(); trx.clear()

#define PREP_ACTOR(name) \
   fc::ecc::private_key name ## _private_key = generate_private_key(BOOST_PP_STRINGIZE(name));   \
   public_key_type name ## _public_key = name ## _private_key.get_public_key();

#define ACTOR(name) \
   PREP_ACTOR(name) \
   const auto& name = create_new_account(get_registrar_id(), BOOST_PP_STRINGIZE(name), name ## _public_key); \
   account_id_type name ## _id = name.id; (void)name ## _id;

#define GET_ACTOR(name) \
   fc::ecc::private_key name ## _private_key = generate_private_key(BOOST_PP_STRINGIZE(name)); \
   const account_object& name = get_account(BOOST_PP_STRINGIZE(name)); \
   account_id_type name ## _id = name.id; \
   (void)name ##_id

#define ACTORS_IMPL(r, data, elem) ACTOR(elem)
#define ACTORS(names) BOOST_PP_SEQ_FOR_EACH(ACTORS_IMPL, ~, names)

#define VAULT_ACTOR(name) \
   PREP_ACTOR(name) \
   const auto& name = create_new_vault_account(get_registrar_id(), BOOST_PP_STRINGIZE(name), name ## _public_key); \
   account_id_type name ## _id = name.id; (void)name ## _id;

#define VAULT_ACTORS_IMPL(r, data, elem) VAULT_ACTOR(elem)
#define VAULT_ACTORS(names) BOOST_PP_SEQ_FOR_EACH(VAULT_ACTORS_IMPL, ~, names)

#define CUSTODIAN_ACTOR(name) \
   PREP_ACTOR(name) \
   const auto& name = create_new_custodian_account(get_registrar_id(), BOOST_PP_STRINGIZE(name), name ## _public_key); \
   account_id_type name ## _id = name.id; (void)name ## _id;

#define CUSTODIAN_ACTORS_IMPL(r, data, elem) CUSTODIAN_ACTOR(elem)
#define CUSTODIAN_ACTORS(names) BOOST_PP_SEQ_FOR_EACH(CUSTODIAN_ACTORS_IMPL, ~, names)

namespace graphene { namespace chain {

struct database_fixture {
   // the reason we use an app is to exercise the indexes of built-in
   //   plugins
   graphene::app::application app;
   genesis_state_type genesis_state;
   chain::database &db;
   database_access_layer _dal;
   signed_transaction trx;
   public_key_type committee_key;
   account_id_type committee_account;
   fc::ecc::private_key private_key = fc::ecc::private_key::generate();
   fc::ecc::private_key init_account_priv_key = fc::ecc::private_key::regenerate(fc::sha256::hash(string("null_key")) );
   public_key_type init_account_pub_key;

   optional<fc::temp_directory> data_dir;
   bool skip_key_index_test = false;
   uint32_t anon_acct_count;

   static constexpr uint32_t apply_bonus(uint32_t value, uint32_t bonus);

   database_fixture();
   ~database_fixture() noexcept(false);

   void init_genesis_state();

   static fc::ecc::private_key generate_private_key(string seed);
   string generate_anon_acct_name();
   static void verify_asset_supplies( const database& db );
   void verify_account_history_plugin_index( )const;
   void open_database();
   signed_block generate_block(uint32_t skip = ~0,
                               const fc::ecc::private_key& key = generate_private_key("null_key"),
                               int miss_blocks = 0);

   /**
    * @brief Generates block_count blocks
    * @param block_count number of blocks to generate
    */
   void generate_blocks(uint32_t block_count);

   /**
    * @brief Generates blocks until the head block time matches or exceeds timestamp
    * @param timestamp target time to generate blocks until
    */
   void generate_blocks(fc::time_point_sec timestamp, bool miss_intermediate_blocks = true, uint32_t skip = ~0);
   
   void do_op(const operation& op) { push_op(op, true); }
   void do_op_no_balance_check(const operation& op) { push_op_no_balance_check(op, true); }
   void push_op(const operation& op, bool gen_block = false);
   void push_op_no_balance_check(const operation& op, bool gen_block = false);

   account_create_operation make_account(
      const account_kind kind,
      const account_id_type registrar,
      const std::string& name = "nathan",
      public_key_type = public_key_type()
      );

   account_create_operation make_account(
      const std::string& name,
      const account_object& registrar,
      const account_object& referrer,
      uint8_t referrer_percent = 100,
      public_key_type key = public_key_type()
      );

   void force_global_settle(const asset_object& what, const price& p);
   operation_result force_settle(account_id_type who, asset what)
   { return force_settle(who(db), what); }
   operation_result force_settle(const account_object& who, asset what);
   void update_feed_producers(asset_id_type mia, flat_set<account_id_type> producers)
   { update_feed_producers(mia(db), producers); }
   void update_feed_producers(const asset_object& mia, flat_set<account_id_type> producers);
   void publish_feed(asset_id_type mia, account_id_type by, const price_feed& f)
   { publish_feed(mia(db), by(db), f); }
   void publish_feed(const asset_object& mia, const account_object& by, const price_feed& f);
   const call_order_object* borrow(account_id_type who, asset what, asset collateral)
   { return borrow(who(db), what, collateral); }
   const call_order_object* borrow(const account_object& who, asset what, asset collateral);
   void cover(account_id_type who, asset what, asset collateral_freed)
   { cover(who(db), what, collateral_freed); }
   void cover(const account_object& who, asset what, asset collateral_freed);

   const asset_object& get_asset( const string& symbol )const;
   const account_object& get_account( const string& name )const;
   const asset_object& create_bitasset(const string& name,
                                       account_id_type issuer = GRAPHENE_WITNESS_ACCOUNT,
                                       uint16_t market_fee_percent = 100 /*1%*/,
                                       uint16_t flags = charge_market_fee);
   const asset_object& create_prediction_market(const string& name,
                                       account_id_type issuer = GRAPHENE_WITNESS_ACCOUNT,
                                       uint16_t market_fee_percent = 100 /*1%*/,
                                       uint16_t flags = charge_market_fee);
   const asset_object& create_user_issued_asset( const string& name );
   const asset_object& create_user_issued_asset( const string& name,
                                                 const account_object& issuer,
                                                 uint16_t flags );
   void issue_uia( const account_object& recipient, asset amount );
   void issue_uia( account_id_type recipient_id, asset amount );

   const account_object& create_account(
      const account_id_type registrar,
      const string& name,
      const public_key_type& key = public_key_type()
      );

   const account_object& create_account(
      const string& name,
      const account_object& registrar,
      const account_object& referrer,
      uint8_t referrer_percent = 100,
      const public_key_type& key = public_key_type()
      );

   const account_object& create_account(
      const string& name,
      const private_key_type& key,
      const account_id_type& registrar_id = account_id_type(),
      const account_id_type& referrer_id = account_id_type(),
      uint8_t referrer_percent = 100
      );

   const account_object& create_vault_account(
      const account_id_type registrar,
      const string& name,
      const public_key_type& key = public_key_type()
      );

   const committee_member_object& create_committee_member( const account_object& owner );
   const witness_object& create_witness(account_id_type owner,
                                        const fc::ecc::private_key& signing_private_key = generate_private_key("null_key"));
   const witness_object& create_witness(const account_object& owner,
                                        const fc::ecc::private_key& signing_private_key = generate_private_key("null_key"));
   uint64_t fund( const account_object& account, const asset& amount = asset(500000) );
   digest_type digest( const transaction& tx );
   void sign( signed_transaction& trx, const fc::ecc::private_key& key );
   const limit_order_object* create_sell_order( account_id_type user, const asset& amount, const asset& recv, share_type reserved = 0 );
   const limit_order_object* create_sell_order( const account_object& user, const asset& amount, const asset& recv, share_type reserved = 0 );
   asset cancel_limit_order( const limit_order_object& order );
   void transfer( account_id_type from, account_id_type to, const asset& amount, const asset& fee = asset() );
   void transfer( const account_object& from, const account_object& to, const asset& amount, const asset& fee = asset() );
   void fund_fee_pool( const account_object& from, const asset_object& asset_to_fund, const share_type amount );
   void enable_fees();
   void change_fees( const flat_set< fee_parameters >& new_params, uint32_t new_scale = 0 );
   void upgrade_to_lifetime_member( account_id_type account );
   void upgrade_to_lifetime_member( const account_object& account );
   void upgrade_to_annual_member( account_id_type account );
   void upgrade_to_annual_member( const account_object& account );
   void print_market( const string& syma, const string& symb )const;
   string pretty( const asset& a )const;
   void print_limit_order( const limit_order_object& cur )const;
   void print_call_orders( )const;
   void print_joint_market( const string& syma, const string& symb )const;
   int64_t get_balance( account_id_type account, asset_id_type a )const;
   int64_t get_balance( const account_object& account, const asset_object& a )const;
   int64_t get_reserved_balance( account_id_type account, asset_id_type a )const;
   int64_t get_dascoin_balance( account_id_type account ) const { return get_balance(account, get_dascoin_asset_id()); }
   vector< operation_history_object > get_operation_history( account_id_type account_id )const;

   // fix_accounts.cpp
   const account_object& make_new_account_base(const account_kind kind, const account_id_type registrar,
                                               const string& name, const public_key_type& key = public_key_type());
   // Use this method to create accounts for DAS tests.
   const account_object& create_new_account(const account_id_type registrar, const string& name,
                                            const public_key_type& key = public_key_type());
   // Use this method to create vault accounts for DAS tests.
   const account_object& create_new_vault_account(const account_id_type registrar, const string& name,
                                                  const public_key_type& key = public_key_type());
  // Use this method to create custodianaccounts for DAS tests.
  const account_object& create_new_custodian_account(const account_id_type registrar, const string& name,
                                                     const public_key_type& key = public_key_type());

   // fix_licenses.cpp
   const license_type_object& create_license_type(const string& kind, const string& name, share_type amount, 
                                                  upgrade_multiplier_type balance_multipliers,
                                                  upgrade_multiplier_type requeue_multipliers,
                                                  upgrade_multiplier_type return_multipliers,
                                                  share_type eur_limit);

   // fix_cycles.cpp
   share_type get_cycle_balance(const account_id_type owner) const;
   void adjust_cycles(const account_id_type id, const share_type amount);

   // fix_getter.cpp
   const global_property_object& get_global_properties() const;
   const dynamic_global_property_object& get_dynamic_global_properties() const;
   const chain_parameters& get_chain_parameters() const;
   account_id_type get_license_administrator_id() const;
   account_id_type get_license_issuer_id() const;
   account_id_type get_webasset_issuer_id() const;
   account_id_type get_webasset_authenticator_id() const;
   account_id_type get_cycle_issuer_id() const;
   account_id_type get_cycle_authenticator_id() const;
   account_id_type get_registrar_id() const;
   account_id_type get_pi_validator_id() const;
   account_id_type get_wire_out_handler_id() const;
   account_id_type get_daspay_administrator_id() const;
   account_id_type get_das33_administrator_id() const;
   asset_id_type get_web_asset_id() const;
   asset_id_type get_cycle_asset_id() const;
   asset_id_type get_dascoin_asset_id() const;
   asset_id_type get_btc_asset_id() const;
   frequency_type get_global_frequency() const;
   const global_property_object::daspay& get_daspay_parameters() const;

   // fix_accounts.cpp
   void tether_accounts(account_id_type wallet, account_id_type vault);
   const account_balance_object& get_account_balance_object(account_id_type account_id, asset_id_type aset_id);
   void set_vault_to_wallet_limit_toggle(account_id_type account_id, bool flag);
   void enable_vault_to_wallet_limit(account_id_type account_id);
   void disable_vault_to_wallet_limit(account_id_type account_id);
   void set_roll_back_enabled(account_id_type account_id, bool roll_back_enabled);
   void roll_back_public_keys(account_id_type authority, account_id_type account_id);

   // fix_daspay.cpp
   vector<payment_service_provider_object> get_payment_service_providers() const;
   void set_daspay_clearing_enabled(bool state);

   // fix_das33.cpp
   vector<das33_project_object> get_das33_projects() const;
   vector<das33_pledge_holder_object> get_das33_pledges() const;

   // fix_web_assets.cpp
   asset web_asset(share_type amount);
   const issued_asset_record_object* issue_asset(const string& unique_id, account_id_type receiver_id,
                                                 share_type cash, share_type reserved,
                                                 asset_id_type asset, account_id_type issuer, string comment);
   const issued_asset_record_object* issue_webasset(const string& unique_id, account_id_type receiver_id, 
                                                    share_type cash, share_type reserved);
   const issued_asset_record_object* issue_cycleasset(const string& unique_id, account_id_type receiver_id,
                                                    share_type cash, share_type reserved);
    const issued_asset_record_object* issue_btcasset(const string& unique_id, account_id_type receiver_id,
                                                       share_type cash, share_type reserved);
   void deny_issue_request(issue_asset_request_id_type request_id);
   std::pair<share_type, share_type> get_web_asset_amounts(account_id_type owner_id);
   std::pair<asset, asset> get_web_asset_balances(account_id_type owner_id);
   void transfer_webasset_vault_to_wallet(account_id_type vault_id, account_id_type wallet_id,
                                          std::pair<share_type, share_type> amounts);
   void transfer_webasset_wallet_to_vault(account_id_type walelt_id, account_id_type vault_id,
                                          std::pair<share_type, share_type> amounts);
   void transfer_dascoin_vault_to_wallet(account_id_type vault_id, account_id_type wallet_id, share_type amount);
   void transfer_dascoin_wallet_to_vault(account_id_type wallet_id, account_id_type vault_id, share_type amount);
   vector<issue_asset_request_object> get_asset_request_objects(account_id_type account_id);
   share_type get_asset_current_supply(asset_id_type asset_id);
   share_type get_web_asset_current_supply() { return get_asset_current_supply(get_web_asset_id()); }
   void set_external_btc_price(price val);
   void set_last_dascoin_price(price val);
   void set_external_bitcoin_price(price val);
   void set_last_daily_dascoin_price(price val);
   void issue_dascoin(account_id_type vault_id, share_type amount);
   void issue_dascoin(account_object& vault_obj, share_type amount);
   void mint_all_dascoin_from_license(license_type_id_type license, account_id_type vault_id, account_id_type wallet_id = account_id_type(),
                                      share_type bonus = 10, share_type frequency_lock = 200);
   asset_id_type create_new_asset(const string& symbol, share_type max_supply, uint8_t precision, const price& core_exchange_rate);

   // fix_pi_limits.cpp
   void update_pi_limits(account_id_type account_id, uint8_t level, limits_type new_limits);

   // fix_wire_out.cpp
   vector<wire_out_holder_object> get_wire_out_holders(account_id_type account_id,
                                                       const flat_set<asset_id_type>& asset_ids) const;
   const wire_out_holder_object& wire_out(account_id_type account_id_type, asset amount, const string& memo = "");
   void wire_out_complete(wire_out_holder_id_type holder_id);
   void wire_out_reject(wire_out_holder_id_type holder_id);

  // fix_wire_out_with_fee.cpp
  vector<wire_out_with_fee_holder_object> get_wire_out_with_fee_holders(account_id_type account_id,
                                                      const flat_set<asset_id_type>& asset_ids) const;
  const wire_out_with_fee_holder_object& wire_out_with_fee(account_id_type account_id_type, asset amount,
                                                           const string& currency_of_choice, const string& to_address,
                                                           const string& memo = "");
  void wire_out_with_fee_complete(wire_out_with_fee_holder_id_type holder_id);
  void wire_out_with_fee_reject(wire_out_with_fee_holder_id_type holder_id);

   // fix_queue.cpp
   void adjust_frequency(frequency_type f);
   void adjust_dascoin_reward(share_type amount);
   void toggle_reward_queue(bool state);
};

namespace test {
/// set a reasonable expiration time for the transaction
void set_expiration( const database& db, transaction& tx );
void set_max_expiration(const database& db, transaction& tx);

bool _push_block( database& db, const signed_block& b, uint32_t skip_flags = 0 );
processed_transaction _push_transaction( database& db, const signed_transaction& tx, uint32_t skip_flags = 0 );

}

} }

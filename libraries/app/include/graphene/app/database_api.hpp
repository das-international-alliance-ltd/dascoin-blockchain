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

#include <graphene/app/full_account.hpp>

#include <graphene/chain/protocol/types.hpp>

#include <graphene/chain/database.hpp>

#include <graphene/chain/access_layer.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/balance_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/confidential_object.hpp>
#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/queue_objects.hpp>
#include <graphene/chain/upgrade_event_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/chain/wire_object.hpp>
#include <graphene/chain/wire_out_with_fee_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/daspay_object.hpp>
#include <graphene/chain/das33_object.hpp>

#include <graphene/market_history/market_history_plugin.hpp>

#include <fc/api.hpp>
#include <fc/optional.hpp>
#include <fc/variant_object.hpp>

#include <fc/network/ip.hpp>

#include <boost/container/flat_set.hpp>

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace graphene { namespace app {

using namespace graphene::chain;
using namespace graphene::market_history;
using namespace std;

class database_api_impl;

struct order
{
   double                     price;
   double                     quote;
   double                     base;
};

struct order_book
{
  string                      base;
  string                      quote;
  vector< order >             bids;
  vector< order >             asks;
};

struct market_ticker
{
   time_point_sec             time;
   string                     base;
   string                     quote;
   double                     latest;
   double                     lowest_ask;
   double                     highest_bid;
   double                     percent_change;
   double                     base_volume;
   double                     quote_volume;
};

struct market_hi_low_volume
{
   string                     base;
   string                     quote;
   double                     high;
   double                     low;
   double                     base_volume;
   double                     quote_volume;
};

struct market_trade
{
   int64_t                    sequence = 0;
   fc::time_point_sec         date;
   double                     price;
   double                     amount;
   double                     value;
   account_id_type            side1_account_id = GRAPHENE_NULL_ACCOUNT;
   account_id_type            side2_account_id = GRAPHENE_NULL_ACCOUNT;
};

// aggregated limit orders with same price
struct aggregated_limit_orders_with_same_price {
   share_type                 price;
   share_type                 base_volume;
   share_type                 quote_volume;
   share_type                 count;
};

// aggregated limit orders grouped by price and devided in two vectors for buy/sell limit orders
struct limit_orders_grouped_by_price
{
   std::vector<aggregated_limit_orders_with_same_price> buy;
   std::vector<aggregated_limit_orders_with_same_price> sell;
};

// aggregated limit orders with same price
struct aggregated_limit_orders_with_same_price_collection {
   share_type                 price;
   share_type                 base_volume;
   share_type                 quote_volume;
   share_type                 count;
   std::vector<aggregated_limit_orders_with_same_price> limit_orders;
};

// aggregated limit orders grouped by price and devided in two vectors for buy/sell limit orders
struct limit_orders_collection_grouped_by_price
{
   std::vector<aggregated_limit_orders_with_same_price_collection> buy;
   std::vector<aggregated_limit_orders_with_same_price_collection> sell;
};

struct cycle_price
{
   share_type                 cycle_amount;
   asset                      asset_amount;
   frequency_type             frequency;
};

struct dasc_holder
{
   account_id_type            holder;
   uint32_t                   vaults;
   share_type                 amount;
};

struct daspay_authority
{
   account_id_type            payment_provider;
   public_key_type            daspay_public_key;
   optional<string>           memo;
};

struct tethered_accounts_balance
{
   account_id_type            account;
   string                     name;
   account_kind               kind;
   share_type                 balance;
   share_type                 reserved;
};

struct tethered_accounts_balances_collection
{
   share_type                 total;
   asset_id_type              asset_id;
   vector<tethered_accounts_balance> details;
};

struct withdrawal_limit
{
   asset limit;
   asset spent;
   time_point_sec start_of_withdrawal;
   time_point_sec last_withdrawal;
};

/**
 * @brief The database_api class implements the RPC API for the chain database.
 *
 * This API exposes accessors on the database which query state tracked by a blockchain validating node. This API is
 * read-only; all modifications to the database must be performed via transactions. Transactions are broadcast via
 * the @ref network_broadcast_api.
 */
class database_api
{
   public:
      database_api( graphene::chain::database& db, const application_options* app_options );
      ~database_api();

      /////////////
      // Objects //
      /////////////

      /**
       * @brief Get the objects corresponding to the provided IDs
       * @param ids IDs of the objects to retrieve
       * @return The objects retrieved, in the order they are mentioned in ids
       *
       * If any of the provided IDs does not map to an object, a null variant is returned in its position.
       */
      fc::variants get_objects(const vector<object_id_type>& ids)const;

      ///////////////////
      // Subscriptions //
      ///////////////////

      void set_subscribe_callback( std::function<void(const variant&)> cb, bool notify_remove_create );
      void set_pending_transaction_callback( std::function<void(const variant&)> cb );
      void set_block_applied_callback( std::function<void(const variant& block_id)> cb );
      /**
       * @brief Stop receiving any notifications
       *
       * This unsubscribes from all subscribed markets and objects.
       */
      void cancel_all_subscriptions();

      /////////////////////////////
      // Blocks and transactions //
      /////////////////////////////

      /**
       * @brief Retrieve a block header
       * @param block_num Height of the block whose header should be returned
       * @return header of the referenced block, or null if no matching block was found
       */
      optional<block_header> get_block_header(uint32_t block_num)const;


      map<uint32_t, optional<block_header>> get_block_header_batch(const vector<uint32_t> block_nums)const;


      /**
       * @brief Retrieve a full, signed block
       * @param block_num Height of the block to be returned
       * @return the referenced block, or null if no matching block was found
       */
      optional<signed_block> get_block(uint32_t block_num)const;

      /**
       * @brief Return an array of full, signed blocks starting from a specified height.
       * @param start_block_num Height of the starting block.
       * @param count Number of blocks to return.
       * @return Array of enumerated blocks
       */
      vector<signed_block_with_num> get_blocks(uint32_t start_block_num, uint32_t count) const;

      /**
       * @brief Return an array of full, signed blocks that contains virtual operations starting from a specified height.
       * @param start_block_num Height of the starting block.
       * @param count Number of blocks to return.
       * @param virtual_operation_ids array of virtual operation ids that should be included in result returned
       * @return Array of enumerated blocks
       */
      vector<signed_block_with_virtual_operations_and_num> get_blocks_with_virtual_operations(uint32_t start_block_num,
                                                                                              uint32_t count,
                                                                                              std::vector<uint16_t> virtual_operation_ids) const;
      /**
       * @brief used to fetch an individual transaction.
       */
      processed_transaction get_transaction( uint32_t start_block_num, uint32_t trx_in_block )const;

      /**
       * If the transaction has not expired, this method will return the transaction for the given ID or
       * it will return NULL if it is not known.  Just because it is not known does not mean it wasn't
       * included in the blockchain.
       */
      optional<signed_transaction> get_recent_transaction_by_id( const transaction_id_type& id )const;

      /////////////
      // Globals //
      /////////////

      /**
       * @brief Retrieve the @ref chain_property_object associated with the chain
       */
      chain_property_object get_chain_properties() const;

      /**
       * @brief Retrieve the current @ref global_property_object
       */
      global_property_object get_global_properties() const;

      /**
       * @brief Retrieve compile-time constants
       */
      fc::variant_object get_config() const;

      /**
       * @brief Get the chain ID
       */
      chain_id_type get_chain_id() const;

      /**
       * @brief Retrieve the current @ref dynamic_global_property_object
       */
      dynamic_global_property_object get_dynamic_global_properties() const;

      /**
       * @brief Get the total amount of cycles and total potential amount of dascoin
       */
      optional<total_cycles_res> get_total_cycles() const;

      //////////
      // Keys //
      //////////

      vector<vector<account_id_type>> get_key_references( vector<public_key_type> key )const;

      //////////////
      // Accounts //
      //////////////

      /**
       * @brief Get a list of accounts by ID
       * @param account_ids IDs of the accounts to retrieve
       * @return The accounts corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<account_object>> get_accounts(const vector<account_id_type>& account_ids)const;

      /**
       * @brief Fetch all objects relevant to the specified accounts and subscribe to updates
       * @param callback Function to call with updates
       * @param names_or_ids Each item must be the name or ID of an account to retrieve
       * @return Map of string from @ref names_or_ids to the corresponding account
       *
       * This function fetches all relevant objects for the given accounts, and subscribes to updates to the given
       * accounts. If any of the strings in @ref names_or_ids cannot be tied to an account, that input will be
       * ignored. All other accounts will be retrieved and subscribed.
       *
       */
      std::map<string,full_account> get_full_accounts( const vector<string>& names_or_ids, bool subscribe );

      optional<account_object> get_account_by_name( string name )const;

      /**
       *  @return all accounts that referr to the key or account id in their owner or active authorities.
       */
      vector<account_id_type> get_account_references( account_id_type account_id )const;

      /**
       * @brief Get a list of accounts by name
       * @param account_names Names of the accounts to retrieve
       * @return The accounts holding the provided names
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<account_object>> lookup_account_names(const vector<string>& account_names)const;

      /**
       * @brief Get names and IDs for registered accounts
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of account names to corresponding IDs
       */
      map<string,account_id_type> lookup_accounts(const string& lower_bound_name, uint32_t limit)const;

      //////////////
      // Balances //
      //////////////

      /**
       * @brief Get an account's balances in various assets
       * @param id ID of the account to get balances for
       * @param assets IDs of the assets to get balances of; if empty, get all assets account has a balance in
       * @return Balances of the account
       */
      vector<asset_reserved> get_account_balances(account_id_type id, const flat_set<asset_id_type>& assets)const;

      /// Semantically equivalent to @ref get_account_balances, but takes a name instead of an ID.
      vector<asset_reserved> get_named_account_balances(const std::string& name, const flat_set<asset_id_type>& assets)const;

      /** @return all unclaimed balance objects for a set of addresses */
      vector<balance_object> get_balance_objects( const vector<address>& addrs )const;

      vector<asset> get_vested_balances( const vector<balance_id_type>& objs )const;

      vector<vesting_balance_object> get_vesting_balances( account_id_type account_id )const;

      /**
       * @brief Get a tethered accounts' balances in various assets
       * @param id ID of the account to get balances for
       * @param assets IDs of the assets to get balances of; if empty, get all assets account has a balance in
       * @return Balances of the tethered accounts
       */
      vector<tethered_accounts_balances_collection> get_tethered_accounts_balances(account_id_type id, const flat_set<asset_id_type>& assets)const;

      /**
       * @brief Get the total number of accounts registered with the blockchain
       */
      uint64_t get_account_count()const;

      ////////////
      // Assets //
      ////////////

      /**
       * @brief Get a list of assets by ID
       * @param asset_ids IDs of the assets to retrieve
       * @return The assets corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<asset_object>> get_assets(const vector<asset_id_type>& asset_ids)const;

      /**
       * @brief Get assets alphabetically by symbol name
       * @param lower_bound_symbol Lower bound of symbol names to retrieve
       * @param limit Maximum number of assets to fetch (must not exceed 100)
       * @return The assets found
       */
      vector<asset_object> list_assets(const string& lower_bound_symbol, uint32_t limit)const;

      /**
       * @brief Get a list of assets by symbol
       * @param symbols_or_ids Symbols or stringified IDs of the assets to retrieve
       * @return The assets corresponding to the provided symbols or IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<asset_object>> lookup_asset_symbols(const vector<string>& symbols_or_ids) const;

      /**
       * @brief Get an asset by symbol
       * @param symbols_or_id Symbol or stringified ID of the asset to retrieve
       * @return The asset corresponding to the provided symbol or ID
       *
       * This function has semantics identical to @ref get_objects
       */
      optional<asset_object> lookup_asset_symbol(const string& symbols_or_id) const;

      /////////////////////
      // Markets / feeds //
      /////////////////////

      /**
       * @brief Get limit orders in a given market
       * @param a ID of asset being sold
       * @param b ID of asset being purchased
       * @param limit Maximum number of orders to retrieve
       * @return The limit orders, ordered from least price to greatest
       */
      vector<limit_order_object>get_limit_orders(asset_id_type a, asset_id_type b, uint32_t limit)const;

      /**
       * @brief Get limit orders in a given market grouped by price and devided in buy and sell vectors
       * @param a ID of asset being sold
       * @param b ID of asset being purchased
       * @param limit Maximum number of orders groups to retrieve per buy and per sell vector
       * @return The limit orders aggregated by same price, ordered by price (in buy - descending in sell - ascending)
       */
      limit_orders_grouped_by_price get_limit_orders_grouped_by_price(asset_id_type a, asset_id_type b, uint32_t limit)const;

      /**
       * @brief Get limit orders in a given market grouped by price and devided in buy and sell vectors
       * @param a ID of asset being sold
       * @param b ID of asset being purchased
       * @param limit Maximum number of orders groups to retrieve per buy and per sell vector
       * @return The limit orders aggregated by same price, ordered by price (in buy - descending in sell - ascending)
       */
      limit_orders_collection_grouped_by_price get_limit_orders_collection_grouped_by_price(asset_id_type a, asset_id_type b, uint32_t limit_group, uint32_t limit_per_group)const;

      /**
       * @brief Get limit orders for an account, in a given market
       * @param id ID of the account to get limit orders for
       * @param a ID of asset being sold
       * @param b ID of asset being purchased
       * @param limit Maximum number of orders to retrieve
       * @return The limit orders, ordered from least price to greatest
       */
      vector<limit_order_object>get_limit_orders_for_account(account_id_type id, asset_id_type a, asset_id_type b, uint32_t limit)const;

    /**
       * @brief Get call orders in a given asset
       * @param a ID of asset being called
       * @param limit Maximum number of orders to retrieve
       * @return The call orders, ordered from earliest to be called to latest
       */
      vector<call_order_object> get_call_orders(asset_id_type a, uint32_t limit)const;

      /**
       * @brief Get forced settlement orders in a given asset
       * @param a ID of asset being settled
       * @param limit Maximum number of orders to retrieve
       * @return The settle orders, ordered from earliest settlement date to latest
       */
      vector<force_settlement_object> get_settle_orders(asset_id_type a, uint32_t limit)const;

      /**
       *  @return all open margin positions for a given account id.
       */
      vector<call_order_object> get_margin_positions( const account_id_type& id )const;

      /**
       * @brief Request notification when the active orders in the market between two assets changes
       * @param callback Callback method which is called when the market changes
       * @param a First asset ID
       * @param b Second asset ID
       *
       * Callback will be passed a variant containing a vector<pair<operation, operation_result>>. The vector will
       * contain, in order, the operations which changed the market, and their results.
       */
      void subscribe_to_market(std::function<void(const variant&)> callback,
                   asset_id_type a, asset_id_type b);

      /**
       * @brief Unsubscribe from updates to a given market
       * @param a First asset ID
       * @param b Second asset ID
       */
      void unsubscribe_from_market( asset_id_type a, asset_id_type b );

      /**
       * @brief Returns the ticker for the market assetA:assetB
       * @param a String name of the first asset
       * @param b String name of the second asset
       * @return The market ticker for the past 24 hours.
       */
      market_ticker get_ticker( const string& base, const string& quote )const;

      /**
       * @brief Returns the 24 hour high, low and volume for the market assetA:assetB
       * @param a String name of the first asset
       * @param b String name of the second asset
       * @return The market high, low and volume over the past 24 hours
       */
      market_hi_low_volume get_24_hi_low_volume( const string& base, const string& quote )const;

      /**
       * @brief Returns the order book for the market base:quote
       * @param base String name of the first asset
       * @param quote String name of the second asset
       * @param depth of the order book. Up to depth of each asks and bids, capped at 50. Prioritizes most moderate of each
       * @return Order book of the market
       */
      order_book get_order_book( const string& base, const string& quote, unsigned limit = 50 )const;

      /**
       * @brief Returns recent trades for the market assetA:assetB, ordered by time, most recent first. The range is [stop, start)
       * Note: Currently, timezone offsets are not supported. The time must be UTC.
       * @param a String name of the first asset
       * @param b String name of the second asset
       * @param stop Stop time as a UNIX timestamp, the earliest trade to retrieve
       * @param limit Number of trasactions to retrieve, capped at 100
       * @param start Start time as a UNIX timestamp, the latest trade to retrieve
       * @return Recent transactions in the market
       */
      vector<market_trade> get_trade_history( const string& base, const string& quote, fc::time_point_sec start, fc::time_point_sec stop, unsigned limit = 100 )const;

      /**
       * @brief Returns trades for the market assetA:assetB, ordered by time, most recent first. The range is [stop, start)
       * Note: Currently, timezone offsets are not supported. The time must be UTC.
       * @param a String name of the first asset
       * @param b String name of the second asset
       * @param stop Stop time as a UNIX timestamp, the earliest trade to retrieve
       * @param limit Number of trasactions to retrieve, capped at 100
       * @param start Start sequence as an Integer, the latest trade to retrieve
       * @return Transactions in the market
       */
      vector<market_trade> get_trade_history_by_sequence( const string& base, const string& quote, int64_t start, fc::time_point_sec stop, unsigned limit = 100 )const;

      /**
       * @brief Check if an asset issue with the corresponding unique was completed on the chain.
       * @param unique_id The unique indentifier string for a single issue
       * @param asset The name or id of the asset.
       * @return True if the issue has been completed, false otherwise.
       **/
      bool check_issued_asset(const string& unique_id, const string& asset) const;

      /**
       * @brief Check if a webeur issue with the corresponding unique was completed on the chain.
       * @param unique_id The unique indentifier string for a single issue
       * @return True if the issue has been completed, false otherwise.
       **/
      bool check_issued_webeur(const string& unique_id) const;

      ///////////////
      // Witnesses //
      ///////////////

      /**
       * @brief Get a list of witnesses by ID
       * @param witness_ids IDs of the witnesses to retrieve
       * @return The witnesses corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<witness_object>> get_witnesses(const vector<witness_id_type>& witness_ids)const;

      /**
       * @brief Get the witness owned by a given account
       * @param account The ID of the account whose witness should be retrieved
       * @return The witness object, or null if the account does not have a witness
       */
      fc::optional<witness_object> get_witness_by_account(account_id_type account)const;

      /**
       * @brief Get names and IDs for registered witnesses
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of witness names to corresponding IDs
       */
      map<string, witness_id_type> lookup_witness_accounts(const string& lower_bound_name, uint32_t limit)const;

      /**
       * @brief Get the total number of witnesses registered with the blockchain
       */
      uint64_t get_witness_count()const;

      ///////////////////////
      // Committee members //
      ///////////////////////

      /**
       * @brief Get a list of committee_members by ID
       * @param committee_member_ids IDs of the committee_members to retrieve
       * @return The committee_members corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<committee_member_object>> get_committee_members(const vector<committee_member_id_type>& committee_member_ids)const;

      /**
       * @brief Get the committee_member owned by a given account
       * @param account The ID of the account whose committee_member should be retrieved
       * @return The committee_member object, or null if the account does not have a committee_member
       */
      fc::optional<committee_member_object> get_committee_member_by_account(account_id_type account)const;

      /**
       * @brief Get names and IDs for registered committee_members
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of committee_member names to corresponding IDs
       */
      map<string, committee_member_id_type> lookup_committee_member_accounts(const string& lower_bound_name, uint32_t limit)const;


      /// WORKERS

      /**
       * Return the worker objects associated with this account.
       */
      vector<worker_object> get_workers_by_account(account_id_type account)const;

      ////////////////////////////
      // Authority / validation //
      ////////////////////////////

      /// @brief Get a hexdump of the serialized binary form of a transaction
      std::string get_transaction_hex(const signed_transaction& trx)const;

      /**
       *  This API will take a partially signed transaction and a set of public keys that the owner has the ability to sign for
       *  and return the minimal subset of public keys that should add signatures to the transaction.
       */
      set<public_key_type> get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const;

      /**
       *  This method will return the set of all public keys that could possibly sign for a given transaction.  This call can
       *  be used by wallets to filter their set of public keys to just the relevant subset prior to calling @ref get_required_signatures
       *  to get the minimum subset.
       */
      set<public_key_type> get_potential_signatures( const signed_transaction& trx )const;
      set<address> get_potential_address_signatures( const signed_transaction& trx )const;

      /**
       * @return true of the @ref trx has all of the required signatures, otherwise throws an exception
       */
      bool           verify_authority( const signed_transaction& trx )const;

      /**
       * @return true if the signers have enough authority to authorize an account
       */
      bool           verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const;

      /**
       *  Validates a transaction against the current state without broadcasting it on the network.
       */
      processed_transaction validate_transaction( const signed_transaction& trx )const;

      /**
       *  For each operation calculate the required fee in the specified asset type.  If the asset type does
       *  not have a valid core_exchange_rate
       */
      vector< fc::variant > get_required_fees( const vector<operation>& ops, asset_id_type id )const;

      ///////////////////////////
      // Proposed transactions //
      ///////////////////////////

      /**
       *  @return the set of proposed transactions relevant to the specified account id.
       */
      vector<proposal_object> get_proposed_transactions( account_id_type id )const;

      //////////////////////
      // Blinded balances //
      //////////////////////

      /**
       *  @return the set of blinded balance objects by commitment ID
       */
      vector<blinded_balance_object> get_blinded_balances( const flat_set<commitment_type>& commitments )const;

      //////////////
      // Licenses //
      //////////////

      /**
       * @brief Get license type-ids found on the blockchain.
       * @param license_type_id_type-id used on the blockchain.
       * @return The license type-id if found
       */
      optional<license_type_object> get_license_type(license_type_id_type) const;

      /**
       * @brief Get all license type-ids found on the blockchain.
       * @return Vector of license type-ids found
       */
      vector<license_type_object> get_license_types() const;

      /**
       * @brief Get all name/license type-ids found on the blockchain.
       * @return Vector of license name/type-ids pairs found
       */
      vector<pair<string, license_type_id_type>> get_license_type_names_ids() const;

      /**
       * @brief Get all license type-ids grouped by kind found on the blockchain.
       * @return Vector of license type-ids found grouped by kind
       */
      vector<license_types_grouped_by_kind_res> get_license_type_names_ids_grouped_by_kind() const;

      /**
       * @brief Get all license objects grouped by kind found on the blockchain.
       * @return Vector of license objects found grouped by kind
       */
      vector<license_objects_grouped_by_kind_res> get_license_objects_grouped_by_kind() const;

      /**
       * @brief Get license types active on the blockchain by name.
       * @param lower_bound_symbol Lower bound of license type names to retrieve
       * @param limit Maximum number of license types to fetch (must not exceed 100)
       * @return The license types found
       */
      vector<license_type_object> list_license_types_by_name(const string& lower_bound_name, uint32_t limit)const;

      /**
       * @brief Get license types active on the blockchain by amount.
       * @param lower_bound_symbol Lower bound of license type names to retrieve.
       * @param limit Maximum number of license types to fetch (must not exceed 100).
       *
       * @return The license types found.
       */
      vector<license_type_object> list_license_types_by_amount(const uint32_t lower_bound_amount, uint32_t limit)const;

      /**
       * @brief Get a list of license types by names
       * @param asset_symbols Symbols or stringified IDs of the assets to retrieve
       * @return The assets corresponding to the provided symbols or IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<license_type_object>> lookup_license_type_names(const vector<string>& names_or_ids)const;

      /**
       * @brief Get a list of account issued license types
       * @param asset_symbols IDs of the accounts to retrieve
       * @return Vector of issued license information objects
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<license_information_object>> get_license_information(const vector<account_id_type>& account_ids) const;

      /**
       * @brief Get a list of upgrade events
       * @return A list of upgrade events, scheduled or executed
       *
       */
      vector<upgrade_event_object> get_upgrade_events() const;

      /////////////
      // Access  //
      /////////////

      /**
       * @brief (Deprecated) Get a free cycle amount for account
       * @param account_ids ID of the account to retrieve
       * @return Number of issued free cycles on account
       *
       */
      acc_id_share_t_res get_free_cycle_balance(account_id_type account_id) const;

      /**
       * @brief (Deprecated) Get cycle amounts per cycle agreement for account
       * @param account_ids ID of the account to retrieve
       * @return Vector of cycle amounts and frequency locks on account
       *
       */
      acc_id_vec_cycle_agreement_res get_all_cycle_balances(account_id_type account_id) const;

      /**
       * @brief Get amount of DASCoin for on an account
       * @return An object containing id and balance of an account
       */
      acc_id_share_t_res get_dascoin_balance(account_id_type id) const;

      /**
       * @brief (Deprecated) Get remaining amount of cycles
       * @param ids Vector of account ids
       * @return Vector of objects containing account id and cycle balance
       */
      vector<acc_id_share_t_res> get_free_cycle_balances_for_accounts(vector<account_id_type> ids) const;

      /**
       * @brief (Deprecated) Get cycle balances for list of accounts
       * @param ids Vector of account ids
       * @return Vector of objects containing account id, cycle amount and frequency lock
       */
      vector<acc_id_vec_cycle_agreement_res> get_all_cycle_balances_for_accounts(vector<account_id_type> ids) const;

      /**
       * @brief Get DASCoin balances for a list of accounts
       * @param ids Vector of account ids
       * @return Vector of object containing id and balance of an account
       */
      vector<acc_id_share_t_res> get_dascoin_balances_for_accounts(vector<account_id_type> ids) const;

      /**
       * @brief Return the entire reward queue.
       * @return Vector of all reward queue objects.
       */
      vector<reward_queue_object> get_reward_queue() const;

      /**
       * @brief Return a portion of the reward queue.
       * @param from Starting page
       * @param amount Number of pages to get
       * @return Vector which represent a portion of that queue
       */
      vector<reward_queue_object> get_reward_queue_by_page(uint32_t from, uint32_t amount) const;

      /**
       * @brief Get the size of the DASCoin reward queue.
       * @return Number of elements in the DASCoin queue.
       */
      uint32_t get_reward_queue_size() const;

      /**
       * @brief Get all current submissions to reward queue by single account
       * @param account_id id of account whose submissions shoud be returned
       * @return All elements on DASCoin reward queue submitted by given account
       */
      acc_id_queue_subs_w_pos_res get_queue_submissions_with_pos(account_id_type account_id) const;

      /**
       * @brief Get all current submissions to reward queue by multiple account
       * @param ids vector of account ids
       * @return All elements on DASCoin reward queue submitted by given accounts
       */
      vector<acc_id_queue_subs_w_pos_res> get_queue_submissions_with_pos_for_accounts(vector<account_id_type> ids) const;

      //////////////////////////
      // REQUESTS:            //
      //////////////////////////

      /**
       * @brief Get all webasset issue request objects, sorted by expiration.
       * @return Vector of webasset issue request objects.
       */
      vector<issue_asset_request_object> get_all_webasset_issue_requests() const;

      /**
       * @brief Get all wire out holder objects.
       * @return Vector of wire out holder objects.
       */
      vector<wire_out_holder_object> get_all_wire_out_holders() const;

      /**
       * @brief Get all wire out with fee holder objects.
       * @return Vector of wire out with fee holder objects.
       */
      vector<wire_out_with_fee_holder_object> get_all_wire_out_with_fee_holders() const;

      /**
       * @brief Get vault information.
       * @param vault_id
       * @return vault_info_res (optional)
       */
      optional<vault_info_res> get_vault_info(account_id_type vault_id) const;

      /**
       * @brief Get vault information for a list of vaults.
       * @param vault_ids A list of vault ID's.
       * @result A JSON object containig a vault id and optional vault information (if vault exists).
       */
      vector<acc_id_vault_info_res> get_vaults_info(vector<account_id_type> vault_ids) const;

      /**
       * @brief Calculates and returns the amount of asset one needs to pay to get the given amount of cycles
       * @param cycle_amount Desired amount of cycles to get
       * @param asset_id Asset to pay
       * @return cycle_price structure (optional)
       */
      optional<cycle_price> calculate_cycle_price(share_type cycle_amount, asset_id_type asset_id) const;

      /**
       * @brief Calculates and returns the list of top 100 dascoin holders.
       * @return Vector of dasc_holder objects.
       */
      vector<dasc_holder> get_top_dasc_holders() const;

      optional<withdrawal_limit> get_withdrawal_limit(account_id_type account, asset_id_type asset_id) const;

      //////////////////////////
      // DASPAY:              //
      //////////////////////////

      /**
       * @brief Get all clearing accounts for all payment service providers.
       * @return List of payment service provider accounts with their respective clearing accounts.
       */
      vector<payment_service_provider_object> get_payment_service_providers() const;

      /**
       * @brief Get daspay authority data for a specified account
       * @return daspay_authority structure (optional)
       */
      optional<vector<daspay_authority>> get_daspay_authority_for_account(account_id_type account) const;

      /**
       * @brief Get all delayed operations for a specified account
       * @param account id of account
       * @return vector of delayed operation objects
       */
      vector<delayed_operation_object> get_delayed_operations_for_account(account_id_type account) const;

      //////////////////////////
      // DAS33:               //
      //////////////////////////

      /**
       * @brief Get all das33 pledges
       * @params from pledges starting with this id will be returned
       * @params limit number of pledges to return, max 100
       * @params phase optional - only return pledges from this phase
       * @return vector of das33 pledge objects
       */
      vector<das33_pledge_holder_object> get_das33_pledges(das33_pledge_holder_id_type from, uint32_t limit, optional<uint32_t> phase) const;

      /**
      * @brief Get all das33 pledges made by an account
      * @params account id of account
      * @return result containing vector of das33 pledge objects and some stats
      */
      das33_pledges_by_account_result get_das33_pledges_by_account(account_id_type account) const;

      /**
      * @brief Get das33 pledges for a project
      * @params project id of a project
      * @params from pledges starting with this id will be returned
      * @params limit number of pledges to return, max 100
      * @params phase optional - only return pledges from this phase
      * @return vector of das33 pledge objects
      */
      vector<das33_pledge_holder_object> get_das33_pledges_by_project(das33_project_id_type project, das33_pledge_holder_id_type from, uint32_t limit, optional<uint32_t> phase = NULL) const;

      /**
      * @brief Get all das33 projects
      * @params lower_bound_name projects starting with this name will be returned
      * @params limit number of projects to return, max 100
      * @return vector of das33 project objects
      */
      vector<das33_project_object> get_das33_projects(const string& lower_bound_name, uint32_t limit) const;

      /**
       * @brief Gets a sum of all pledges made to project
       * @params project id of a project
       * @return vector of assets, each with total sum of that asset pledged
       */
      vector<asset> get_amount_of_assets_pledged_to_project(das33_project_id_type project) const;

      /**
       * @brief Gets the amount of project tokens that a pledger can get for pledging a certain amount of asset
       * @params project id of a project
       * @params to_pledge asset user is pledging
       * @return amount of project tokens to get
       */
      das33_project_tokens_amount get_amount_of_project_tokens_received_for_asset(das33_project_id_type project, asset to_pledge) const;

      /**
      * @brief Gets the amount of assets needed to be pledge to get given amount of base project tokens
      * @params project id of a project
      * @params asset_id id of an asset user wants to get amount for
      * @params to_pledge project token user wants to get
      * @return amount of project tokens to get
      */
      das33_project_tokens_amount get_amount_of_asset_needed_for_project_token(das33_project_id_type project, asset_id_type asset_id, asset tokens) const;

      //////////////////////////
      // Prices:              //
      //////////////////////////

      /**
       * @brief Gets all last prices from index
       * @return Vector of last price objects
       */
      vector<last_price_object> get_last_prices() const;

      /**
       * @brief Gets all external prices from external price index
       * @return Vector of external price objects
       */
      vector<external_price_object> get_external_prices() const;
private:
      std::shared_ptr< database_api_impl > my;
};

} }

FC_REFLECT( graphene::app::order, (price)(quote)(base) );
FC_REFLECT( graphene::app::order_book, (base)(quote)(bids)(asks) );
FC_REFLECT( graphene::app::market_ticker, (time)(base)(quote)(latest)(lowest_ask)(highest_bid)(percent_change)(base_volume)(quote_volume) );
FC_REFLECT( graphene::app::market_hi_low_volume, (base)(quote)(high)(low)(base_volume)(quote_volume) );
FC_REFLECT( graphene::app::market_trade, (sequence)(date)(price)(amount)(value) );
FC_REFLECT( graphene::app::aggregated_limit_orders_with_same_price, (price)(base_volume)(quote_volume)(count) );
FC_REFLECT( graphene::app::limit_orders_grouped_by_price, (buy)(sell) );
FC_REFLECT( graphene::app::aggregated_limit_orders_with_same_price_collection, (price)(base_volume)(quote_volume)(count)(limit_orders) );
FC_REFLECT( graphene::app::limit_orders_collection_grouped_by_price, (buy)(sell) );
FC_REFLECT( graphene::app::cycle_price, (cycle_amount)(asset_amount)(frequency) );
FC_REFLECT( graphene::app::dasc_holder, (holder)(vaults)(amount) );
FC_REFLECT( graphene::app::daspay_authority, (payment_provider)(daspay_public_key)(memo) );
FC_REFLECT( graphene::app::tethered_accounts_balance, (account)(name)(kind)(balance)(reserved) );
FC_REFLECT( graphene::app::tethered_accounts_balances_collection, (asset_id)(total)(details) );
FC_REFLECT( graphene::app::withdrawal_limit, (limit)(spent)(start_of_withdrawal)(last_withdrawal) );

FC_API( graphene::app::database_api,
   // Objects
   (get_objects)

   // Subscriptions
   (set_subscribe_callback)
   (set_pending_transaction_callback)
   (set_block_applied_callback)
   (cancel_all_subscriptions)

   // Blocks and transactions
   (get_block_header)
   (get_block)
   (get_blocks)
   (get_blocks_with_virtual_operations)
   (get_transaction)
   (get_recent_transaction_by_id)

   // Globals
   (get_chain_properties)
   (get_global_properties)
   (get_config)
   (get_chain_id)
   (get_dynamic_global_properties)
   (get_total_cycles)

   // Keys
   (get_key_references)

   // Accounts
   (get_accounts)
   (get_full_accounts)
   (get_account_by_name)
   (get_account_references)
   (lookup_account_names)
   (lookup_accounts)
   (get_account_count)

   // Balances
   (get_account_balances)
   (get_named_account_balances)
   (get_balance_objects)
   (get_vested_balances)
   (get_vesting_balances)
   (get_tethered_accounts_balances)

   // Assets
   (get_assets)
   (list_assets)
   (lookup_asset_symbols)
   (lookup_asset_symbol)

   // Markets / feeds
   (get_order_book)
   (get_limit_orders)
   (get_limit_orders_for_account)
   (get_limit_orders_grouped_by_price)
   (get_limit_orders_collection_grouped_by_price)
   (get_call_orders)
   (get_settle_orders)
   (get_margin_positions)
   (subscribe_to_market)
   (unsubscribe_from_market)
   (get_ticker)
   (get_24_hi_low_volume)
   (get_trade_history)
   (get_trade_history_by_sequence)
   (check_issued_asset)
   (check_issued_webeur)

   // Witnesses
   (get_witnesses)
   (get_witness_by_account)
   (lookup_witness_accounts)
   (get_witness_count)

   // Committee members
   (get_committee_members)
   (get_committee_member_by_account)
   (lookup_committee_member_accounts)

   // workers
   (get_workers_by_account)

   // Authority / validation
   (get_transaction_hex)
   (get_required_signatures)
   (get_potential_signatures)
   (get_potential_address_signatures)
   (verify_authority)
   (verify_account_authority)
   (validate_transaction)
   (get_required_fees)

   // Proposed transactions
   (get_proposed_transactions)

   // Blinded balances
   (get_blinded_balances)

   // Licenses
   (get_license_type)
   (get_license_types)
   (get_license_type_names_ids)
   (get_license_type_names_ids_grouped_by_kind)
   (get_license_objects_grouped_by_kind)
   (list_license_types_by_name)
   (list_license_types_by_amount)
   (lookup_license_type_names)
   (get_license_information)
   (get_upgrade_events)

   // Access
   (get_free_cycle_balance)
   (get_all_cycle_balances)
   (get_dascoin_balance)
   (get_free_cycle_balances_for_accounts)
   (get_all_cycle_balances_for_accounts)
   (get_dascoin_balances_for_accounts)

   // Queue
   (get_reward_queue)
   (get_reward_queue_size)
   (get_reward_queue_by_page)
   (get_queue_submissions_with_pos)
   (get_queue_submissions_with_pos_for_accounts)

   // Requests
   (get_all_webasset_issue_requests)
   (get_all_wire_out_holders)
   (get_all_wire_out_with_fee_holders)

   // Vaults
   (get_vault_info)
   (get_vaults_info)

   // Calculate cycle price
   (calculate_cycle_price)

   // Top dascoin holders
   (get_top_dasc_holders)

   (get_withdrawal_limit)

   // DasPay
   (get_payment_service_providers)
   (get_daspay_authority_for_account)
   (get_delayed_operations_for_account)

   // Das33
   (get_das33_pledges)
   (get_das33_pledges_by_account)
   (get_das33_pledges_by_project)
   (get_das33_projects)
   (get_amount_of_assets_pledged_to_project)
   (get_amount_of_project_tokens_received_for_asset)
   (get_amount_of_asset_needed_for_project_token)

   // Prices
   (get_last_prices)
   (get_external_prices)
)

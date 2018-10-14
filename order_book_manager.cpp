#include "order_book_manager.hpp"
#include <iostream>
#include <cstdint>

#define LOW_ACCESS_INDEX 50

OrderBookManager::OrderBookManager(OrderBook &t_order_book)
    : order_book_(t_order_book),
      bid_order_info_map_iter_(),
      order_id_to_bid_order_info_map_(),
      ask_order_info_map_iter_(),
      order_id_to_ask_order_info_map_()
{
}

/*
* This function handles the case when a new order is added to the book
*/
void OrderBookManager::OnOrderAdd(uint64_t t_order_id_, uint8_t t_side_, double t_price_, int t_size_)
{

#if DEBUG_MODE_ON
    std::cout << typeid(*this).name() << ":" << __func__ << " " << t_order_id_ << " [" << t_price_ << "," << t_size_
              << "," << t_side_ << "]" << std::endl;
#endif

    int int_price = order_book_.GetIntPx(t_price_);

    if (!order_book_.initial_book_constructed_)
    {
        order_book_.BuildIndex(t_side_, int_price);
    }

    int new_order_level = 0;

    switch (t_side_)
    {
    case 'B':
    {
        bid_order_info_map_iter_ = order_id_to_bid_order_info_map_.find(t_order_id_);

        if (bid_order_info_map_iter_ == order_id_to_bid_order_info_map_.end())
        {
            OrderInfo new_order(t_price_, t_size_, t_side_);
            order_id_to_bid_order_info_map_[t_order_id_] = new_order;
        }
        else
        {
            std::cout << "Bid Order already present with order_id: " << t_order_id_ << "\n";
            return;
        }

        int bid_index = order_book_.GetBidIndex(int_price);

        // There are 0 levels on bid side
        if (order_book_.IsBidBookEmpty())
        {
            if (bid_index < LOW_ACCESS_INDEX)
            {
                order_book_.RebuildIndexLowAccess(t_side_, int_price);
                bid_index = order_book_.base_bid_index_;
            }
            else if (bid_index >= (int)order_book_.max_tick_range_)
            {
                order_book_.RebuildIndexHighAccess(t_side_, int_price);
                bid_index = order_book_.base_bid_index_;
            }
            order_book_.base_bid_index_ = bid_index;
        }
        else if (bid_index < 0)
        {
            std::cout << " Order added way below the best bid level, Ignoring this order with order_id :"
                      << t_order_id_ << "\n";
            return;
        }

        // new order is at very high price
        if (bid_index >= (int)order_book_.max_tick_range_)
        {
            order_book_.RebuildIndexHighAccess(t_side_, int_price);
            bid_index = order_book_.base_bid_index_;
        }

        if (order_book_.IsBidLevelEmpty(bid_index))
        {
            order_book_.UpdateBidLevel(bid_index, t_size_, 1);
        }
        else
        {
            int cur_size_at_level = order_book_.GetBidSize(bid_index);
            int cur_num_of_orders_at_level = order_book_.GetBidOrders(bid_index);
            order_book_.UpdateBidLevel(bid_index, cur_size_at_level + t_size_, cur_num_of_orders_at_level + 1);
        }

        order_book_.base_bid_index_ = std::max((int)order_book_.base_bid_index_, bid_index);

        // find the level at which the new order is added
        for (int t_index = order_book_.base_bid_index_ - 1; t_index >= bid_index; t_index--)
        {
            if (order_book_.GetBidSize(t_index) > 0 && order_book_.GetBidOrders(t_index) > 0)
                new_order_level++;
        }
    }
    break;
    case 'S':
    {
        ask_order_info_map_iter_ = order_id_to_ask_order_info_map_.find(t_order_id_);

        if (ask_order_info_map_iter_ == order_id_to_ask_order_info_map_.end())
        {
            OrderInfo new_order(t_price_, t_size_, t_side_);
            order_id_to_ask_order_info_map_[t_order_id_] = new_order;
        }
        else
        {
            std::cout << " Ask Order already present with order_id: " << t_order_id_ << "\n";
            return;
        }

        int ask_index = order_book_.GetAskIndex(int_price);

        // There are 0 levels on ask side
        if (order_book_.IsAskBookEmpty())
        {
            if (ask_index < LOW_ACCESS_INDEX)
            {
                order_book_.RebuildIndexLowAccess(t_side_, int_price);
                ask_index = order_book_.base_ask_index_;
            }
            else if (ask_index >= (int)order_book_.max_tick_range_)
            {
                order_book_.RebuildIndexHighAccess(t_side_, int_price);
                ask_index = order_book_.base_ask_index_;
            }

            order_book_.base_ask_index_ = ask_index;
        }
        else if (ask_index < 0)
        {
            std::cout << " Order added way below the best ask level, Ignoring this order with order_id :"
                      << t_order_id_ << "\n";
            return;
        }

        if (ask_index >= (int)order_book_.max_tick_range_)
        {
            order_book_.RebuildIndexHighAccess(t_side_, int_price);
            ask_index = order_book_.base_ask_index_;
        }

        if (order_book_.IsAskLevelEmpty(ask_index))
        {
            order_book_.UpdateAskLevel(ask_index, t_size_, 1);
        }
        else
        {
            int cur_size_at_level = order_book_.GetAskSize(ask_index);
            int cur_num_of_orders_at_level = order_book_.GetAskOrders(ask_index);
            order_book_.UpdateAskLevel(ask_index, cur_size_at_level + t_size_, cur_num_of_orders_at_level + 1);
        }

        if (ask_index >= (int)order_book_.base_ask_index_)
        {
            order_book_.base_ask_index_ = ask_index;
        }

        for (int t_index = order_book_.base_ask_index_ - 1; t_index >= ask_index; t_index--)
        {
            if (order_book_.GetAskSize(t_index) > 0 && order_book_.GetAskOrders(t_index) > 0)
                new_order_level++;
        }
    }
    break;
    default:
    {
        std::cout << " Side is neither B or S: " << t_side_ << "\n";
        return;
    }
    break;
    }

#if DEBUG_MODE_ON
    std::cout << typeid(*this).name() << ":" << __func__ << "Order Added at level :" << new_order_level << "...."
              << std::endl;
#endif
}

void OrderBookManager::OnOrderDelete(uint64_t t_order_id_, uint8_t t_side_)
{

#if DEBUG_MODE_ON
    std::cout << typeid(*this).name() << ':' << __func__ << " " << t_order_id_;
#endif

    if (!order_book_.initial_book_constructed_)
    {
        return;
    }

    int order_size = 0;
    double order_price = 0.0;

    int cumulative_old_size = 0;
    int cumulative_old_ordercount = 0;
    int int_price = 0;
    bool is_level_deleted = false;
    bool is_order_present_in_best_level = false;

    int level_changed = 0;

    switch (t_side_)
    {
    case 'B':
    {
        // searching the bid_order_map to retrieve the meta-data corresponding to @t_order_id
        bid_order_info_map_iter_ = order_id_to_bid_order_info_map_.find(t_order_id_);
        if (bid_order_info_map_iter_ != order_id_to_bid_order_info_map_.end())
        {
            order_price = (bid_order_info_map_iter_->second).price;
            order_size = (bid_order_info_map_iter_->second).size;
            order_id_to_bid_order_info_map_.erase(bid_order_info_map_iter_);
        }
        else
        {
            std::cout << " Error: Bid OrderId: " << t_order_id_ << " not present to delete."
                      << "\n";
            return;
        }
#if DEBUG_MODE_ON
        std::cout << " [" << order_price << "," << order_size << "," << t_side_ << "]" << std::endl;
#endif
        int_price = order_book_.GetIntPx(order_price);

        // find the index in bid_level vector to which this order belongs
        int bid_index = order_book_.GetBidIndex(int_price);

        if (bid_index == (int)order_book_.base_bid_index_)
            is_order_present_in_best_level = true;

        // store the old information corresponding to the above found level @bid_index
        int cumulative_old_size = order_book_.GetBidSize(bid_index);
        int cumulative_old_ordercount = order_book_.GetBidOrders(bid_index);

        // modify the data at level @bid_index
        order_book_.UpdateBidLevel(bid_index, cumulative_old_size - order_size, cumulative_old_ordercount - 1);

        // checking if no more orders are left at @bid_index
        is_level_deleted = order_book_.IsBidLevelEmpty(bid_index);

#if DEBUG_MODE_ON
        if (is_level_deleted)
            std::cout << "Level Deleted...." << std::endl;
#endif

        if (is_level_deleted)
        {
            // checking if the level deleted is the best_bid_level
            if (is_order_present_in_best_level)
            {
                UpdateBaseBidIndex();
            }
        }

        for (int t_index = order_book_.base_bid_index_ - 1; t_index >= bid_index; t_index--)
        {
            if (!order_book_.IsBidLevelEmpty(t_index) || t_index == bid_index) {
                level_changed++;
            }
        }
    }
    break;
    case 'S':
    {
        // searching the ask_order_map to retrieve the meta-data corresponding to @t_order_id

        ask_order_info_map_iter_ = order_id_to_ask_order_info_map_.find(t_order_id_);
        if (ask_order_info_map_iter_ != order_id_to_ask_order_info_map_.end())
        {
            order_price = ask_order_info_map_iter_->second.price;
            order_size = ask_order_info_map_iter_->second.size;
            order_id_to_ask_order_info_map_.erase(ask_order_info_map_iter_);
        }
        else
        {
            std::cout << " Error: Ask OrderId: " << t_order_id_ << " not present to delete."
                      << "\n";
            return;
        }
#if DEBUG_MODE_ON
        std::cout << " [" << order_price << "," << order_size << "," << t_side_ << "]" << std::endl;
#endif
        int_price = order_book_.GetIntPx(order_price);

        // find the index in ask_level vector to which this order belongs
        int ask_index = order_book_.GetAskIndex(int_price);

        if (ask_index == (int)order_book_.base_ask_index_)
            is_order_present_in_best_level = true;

        // store the old information corresponding to the above fund level @ask_index
        cumulative_old_size = order_book_.GetAskSize(ask_index);
        cumulative_old_ordercount = order_book_.GetAskOrders(ask_index);

        // modify the data at level @ask_index
        order_book_.UpdateAskLevel(ask_index, cumulative_old_size - order_size, cumulative_old_ordercount - 1);

        // checking if no more orders are left at @ask_index
        is_level_deleted = (order_book_.IsAskLevelEmpty(ask_index));

#if DEBUG_MODE_ON
        if (is_level_deleted)
            std::cout << "Level Deleted...." << std::endl;
#endif

        if (is_level_deleted)
        {
            // checking if the level deleted is the best_ask_level
            if (is_order_present_in_best_level)
            {
                UpdateBaseAskIndex();
            }
        }

        for (int t_index = order_book_.base_ask_index_ - 1; t_index >= ask_index; t_index--)
        {
            if (!order_book_.IsAskLevelEmpty(t_index) || t_index == ask_index)
                level_changed++;
        }
    }
    break;
    default:
    {
        std::cout << "Invalid Side: " << t_side_ << "\n";
        return;
    }
    break;
    }

#if DEBUG_MODE_ON
    std::cout << typeid(*this).name() << ":" << __func__ << " Order Deleted at level :" << level_changed << "...."
              << std::endl;
#endif
}

/*
 * This function assumes that the order to modify will only have its size changed
 * but prices will remain same as before.The "Replace Order" where both prices and
 * size can change has been implemented in OrderReplace as OrderDelete + OrderAdd
 */
void OrderBookManager::OnOrderModify(uint64_t t_order_id_, uint8_t t_side_, int t_new_size_, uint64_t t_new_order_id_)
{
#if DEBUG_MODE_ON
    std::cout << typeid(*this).name() << ':' << __func__ << " " << t_order_id_;
    std::cout << "[" << t_new_order_id_ << "," << t_new_size_ << "," << t_side_ << "]" << std::endl;
#endif

    double old_order_price = 0.0;
    int old_order_size = 0;

    int int_price = 0;
    int cumulative_old_size = 0;
    int cumulative_old_ordercount = 0;
    int level_modified = 0;

    bool is_level_deleted = false;
    bool is_order_present_in_best_level = false;

    switch (t_side_)
    {
    case 'B':
    {
        bid_order_info_map_iter_ = order_id_to_bid_order_info_map_.find(t_order_id_);
        if (bid_order_info_map_iter_ != order_id_to_bid_order_info_map_.end())
        {
            OrderInfo order_info = bid_order_info_map_iter_->second;
            old_order_price = order_info.price;
            old_order_size = order_info.size;

            // update the size
            order_info.size = t_new_size_;

            order_id_to_bid_order_info_map_.erase(bid_order_info_map_iter_);
            order_id_to_bid_order_info_map_[t_new_order_id_] = order_info;
        }
        else
        {
            std::cout << " Error: BidOrderId: " << t_order_id_ << " not present to replace."
                      << "\n";
            return;
        }

        int_price = order_book_.GetIntPx(old_order_price);

        if (!order_book_.initial_book_constructed_)
        {
            order_book_.BuildIndex(t_side_, int_price);
            order_book_.initial_book_constructed_ = true;
        }

        int bid_index = order_book_.GetBidIndex(int_price);

        is_order_present_in_best_level = (bid_index == (int)order_book_.base_bid_index_);

        cumulative_old_size = order_book_.GetBidSize(bid_index);
        cumulative_old_ordercount = order_book_.GetBidOrders(bid_index);

        order_book_.UpdateBidLevel(bid_index, cumulative_old_size - old_order_size + t_new_size_,
                                   cumulative_old_ordercount);

        for (int t_index = order_book_.base_bid_index_ - 1; t_index >= bid_index; t_index--)
        {
            if (!order_book_.IsBidLevelEmpty(t_index))
                level_modified++;
        }
    }
    break;
    case 'S':
    {
        // checking if the iterator is set already before calling this function
        ask_order_info_map_iter_ = order_id_to_ask_order_info_map_.find(t_order_id_);
        if (ask_order_info_map_iter_ != order_id_to_ask_order_info_map_.end())
        {
            OrderInfo order_info = ask_order_info_map_iter_->second;
            old_order_price = order_info.price;
            old_order_size = order_info.size;

            // update the size
            order_info.size = t_new_size_;

            order_id_to_ask_order_info_map_.erase(ask_order_info_map_iter_);
            order_id_to_ask_order_info_map_[t_new_order_id_] = order_info;
        }
        else
        {
            std::cout << " Error: AskOrderId: " << t_order_id_ << " not present to replace."
                      << "\n";
            return;
        }

        int_price = order_book_.GetIntPx(old_order_price);

        if (!order_book_.initial_book_constructed_)
        {
            order_book_.BuildIndex(t_side_, int_price);
        }

        int ask_index = order_book_.GetAskIndex(int_price);

        // update the size at the corresponding level
        cumulative_old_size = order_book_.GetAskSize(ask_index);
        cumulative_old_ordercount = order_book_.GetAskOrders(ask_index);
        order_book_.UpdateAskLevel(ask_index, cumulative_old_size - old_order_size + t_new_size_, cumulative_old_ordercount);

        // check which level is modified
        for (int t_index = order_book_.base_ask_index_ - 1; t_index >= ask_index; t_index--)
        {
            if (!order_book_.IsAskLevelEmpty(t_index))
                level_modified++;
        }
    }
    break;
    default:
    {
        std::cout << " Invalid Side : " << t_side_
                  << "\n";
        return;
    }
    break;
    }

#if DEBUG_MODE_ON
    std::cout << typeid(*this).name() << ":" << __func__ << "Order modified at level :" << level_modified << "...."
              << std::endl;
#endif
}

/*
 * This is case where both price and size of an order can be changed. If the price of the order is same
 * as before, just call the OrderModify ( which handles modify with same px ). If prices are different
 * then simulate it with a Delete + Add ( if new_size > 0 )
 */
void OrderBookManager::OnOrderReplace(uint64_t t_order_id_, uint8_t t_side_, double t_new_price_,
                                      int t_new_size_, uint64_t t_new_order_id_)
{

#if DEBUG_MODE_ON
    std::cout << typeid(*this).name() << ':' << __func__ << " " << t_order_id_ << std::endl;
#endif

    double old_price;

    if (t_side_ == 'B')
    {
        bid_order_info_map_iter_ = order_id_to_bid_order_info_map_.find(t_order_id_);

        if (bid_order_info_map_iter_ != order_id_to_bid_order_info_map_.end())
        {
            OrderInfo order_info = bid_order_info_map_iter_->second;
            old_price = order_info.price;
        }
        else
        {
            return;
        }
    }
    else if (t_side_ == 'S')
    {
        ask_order_info_map_iter_ = order_id_to_ask_order_info_map_.find(t_order_id_);

        if (ask_order_info_map_iter_ != order_id_to_ask_order_info_map_.end())
        {
            OrderInfo order_info = ask_order_info_map_iter_->second;

            old_price = order_info.price;
        }
        else
        {
            return;
        }
    }
    else
    {
        std::cout << " Invalid Side: " << t_side_ << "\n";
        return;
    }

    int old_int_price = order_book_.GetIntPx(old_price);
    int new_int_price = order_book_.GetIntPx(t_new_price_);

    if (old_int_price == new_int_price)
    {
        OnOrderModify(t_order_id_, t_side_, t_new_size_, t_new_order_id_);
    }
    else if (t_new_size_ > 0)
    {
        OnOrderDelete(t_order_id_, t_side_);
        OnOrderAdd(t_new_order_id_, t_side_, t_new_price_, t_new_size_);
    }
    else
    {
        OnOrderDelete(t_order_id_, t_side_);
    }
}

/*
 * This function assumes that the order exec received has been for a resting order,we
 * simulate it as Delete ( if size is 0 ) or Modify ( if still has some size )
 */
void OrderBookManager::OnOrderExec(uint64_t t_order_id_, uint8_t t_side_, double t_price_, int t_size_exec_)
{
#if DEBUG_MODE_ON
    std::cout << typeid(*this).name() << ':' << __func__ << " " << t_order_id_ << std::endl;
#endif

    // not till book is ready
    if (!order_book_.initial_book_constructed_ || order_book_.IsBidBookEmpty() || order_book_.IsAskBookEmpty())
    {
        return;
    }

    int int_trade_price = order_book_.GetIntPx(t_price_);

    if (!(t_side_ == 'B' || t_side_ == 'S'))
    {
        return;
    }

    int old_order_size = 0;

    // Find the order_id_ in the map, if not preset its an error case, return
    if (t_side_ == 'B')
    {
        bid_order_info_map_iter_ = order_id_to_bid_order_info_map_.find(t_order_id_);

        if (bid_order_info_map_iter_ != order_id_to_bid_order_info_map_.end())
        {
            old_order_size = bid_order_info_map_iter_->second.size;
        }
        else
        {
            std::cout << " Error: BidOrderId: " << t_order_id_ << " not present for Exec."
                      << "\n";
            return;
        }
    }
    else if (t_side_ == 'S')
    {
        ask_order_info_map_iter_ = order_id_to_ask_order_info_map_.find(t_order_id_);

        if (ask_order_info_map_iter_ != order_id_to_ask_order_info_map_.end())
        {
            old_order_size = ask_order_info_map_iter_->second.size;
        }
        else
        {
            std::cout << " Error: AskOrderId: " << t_order_id_ << " not present to Exec."
                      << "\n";
            return;
        }
    }

    int order_size_remained = old_order_size - t_size_exec_;
#if DEBUG_MODE_ON
    std::cout << "[" << old_order_size << "," << order_size_remained << "," << t_side_ << "]" << std::endl;
#endif

    if (order_size_remained > 0)
    {
        OnOrderModify(t_order_id_, t_side_, order_size_remained, t_order_id_);
    }
    else
    {
        OnOrderDelete(t_order_id_, t_side_);
    }
}

void OrderBookManager::OnOrderResetBegin()
{
    std::cout << " Resetting order book, flushing all the orders so far...\n";
    order_book_.Initialize();

    // flushing all the orders
    order_id_to_bid_order_info_map_.clear();
    order_id_to_ask_order_info_map_.clear();
}

void OrderBookManager::UpdateBaseBidIndex()
{
    int next_bid_index_ = order_book_.base_bid_index_ - 1;

    // finding the next best bid index
    for (; next_bid_index_ >= 0; next_bid_index_--)
    {
        if (order_book_.IsBidLevelEmpty(next_bid_index_))
        {
            order_book_.ResetBidLevel(next_bid_index_);
        }
        else
        {
            break;
        }
    }

    if (next_bid_index_ < 0)
    {
        return;
    }

    order_book_.base_bid_index_ = next_bid_index_; // updating the best bid level

    if (order_book_.base_bid_index_ < LOW_ACCESS_INDEX)
    {
        order_book_.RebuildIndexLowAccess('B', order_book_.GetBidIntPrice(next_bid_index_));
    }
}

void OrderBookManager::UpdateBaseAskIndex()
{
    int next_ask_index_ = order_book_.base_ask_index_ - 1;

    // finding the next best ask index
    for (; next_ask_index_ >= 0; next_ask_index_--)
    {
        if (order_book_.IsAskLevelEmpty(next_ask_index_))
        {
            order_book_.ResetAskLevel(next_ask_index_);
        }
        else
        {
            break;
        }
    }

    if (next_ask_index_ < 0)
    {
        return;
    }

    order_book_.base_ask_index_ = next_ask_index_; // updating the best ask level

    if (order_book_.base_ask_index_ < LOW_ACCESS_INDEX)
    {
        order_book_.RebuildIndexLowAccess('S', order_book_.GetAskIntPrice(next_ask_index_));
    }
}
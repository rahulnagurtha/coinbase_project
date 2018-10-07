#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>

#include "order_book.hpp"

#define LOW_ACCESS_INDEX 50
#define MIN_INITIAL_TICK_BASE 1024

OrderBook::OrderBook(std::string t_exchange_symbol_, double min_price_increment)
    : exchange_symbol_(t_exchange_symbol_),
      min_price_increment_(min_price_increment),
      is_ready_(false),
      initial_book_constructed_(false),
      base_bid_index_(0u),
      base_ask_index_(0u),
      initial_tick_size_(MIN_INITIAL_TICK_BASE),
      max_tick_range_(MIN_INITIAL_TICK_BASE)
{
}

void OrderBook::Initialize()
{

    initial_tick_size_ = MIN_INITIAL_TICK_BASE;

    max_tick_range_ = (2 * initial_tick_size_ + 1);

    bid_levels_.clear();
    ask_levels_.clear();

    initial_book_constructed_ = false;

    bid_levels_.resize(max_tick_range_);
    ask_levels_.resize(max_tick_range_);
}

std::string OrderBook::ShowMarket()
{
    std::ostringstream t_temp_oss_;
    t_temp_oss_ << exchange_symbol_ << "\n";
    unsigned int m_m_levels = 5;
    for (unsigned int t_level_ = 0; t_level_ < m_m_levels; t_level_++)
    {
        t_temp_oss_.width(5);
        t_temp_oss_ << bid_size(t_level_);
        t_temp_oss_ << " ";
        t_temp_oss_.width(5);
        t_temp_oss_ << bid_order(t_level_);
        t_temp_oss_ << " ";
        t_temp_oss_.width(6);
        t_temp_oss_ << bid_price(t_level_);
        t_temp_oss_ << " ";
        t_temp_oss_.width(6);
        t_temp_oss_ << bid_int_price(t_level_);
        t_temp_oss_ << " X ";
        t_temp_oss_.width(6);
        t_temp_oss_ << ask_int_price(t_level_);
        t_temp_oss_ << " ";
        t_temp_oss_.width(6);
        t_temp_oss_ << ask_price(t_level_);
        t_temp_oss_ << " ";
        t_temp_oss_.width(5);
        t_temp_oss_ << ask_order(t_level_);
        t_temp_oss_ << " ";
        t_temp_oss_.width(6);
        t_temp_oss_ << ask_size(t_level_);

        t_temp_oss_ << std::endl;
    }
    return t_temp_oss_.str();
}

bool OrderBook::IsBidLevelEmpty(int bid_index)
{
    return GetBidSize(bid_index) <= 0 || GetBidOrders(bid_index) <= 0;
}

bool OrderBook::IsAskLevelEmpty(int ask_index)
{
    return GetAskSize(ask_index) <= 0 || GetAskOrders(ask_index) <= 0;
}

bool OrderBook::IsAskBookEmpty()
{
    return IsAskLevelEmpty(base_ask_index_);
}

bool OrderBook::IsBidBookEmpty()
{
    return IsBidLevelEmpty(base_bid_index_);
}

void OrderBook::UpdateBidLevel(int index, int size, int ordercount)
{
    if (size < 0 || ordercount < 0)
    {
        ResetBidLevel(index);
    }
    else
    {
        bid_levels_[index].limit_size_ = size;
        bid_levels_[index].limit_ordercount_ = ordercount;
    }
}

void OrderBook::UpdateAskLevel(int index, int size, int ordercount)
{
    if (size < 0 || ordercount < 0)
    {
        ResetAskLevel(index);
    }
    else
    {
        ask_levels_[index].limit_size_ = size;
        ask_levels_[index].limit_ordercount_ = ordercount;
    }
}

void OrderBook::ResetBidLevel(int index)
{
    bid_levels_[index].limit_size_ = 0;
    bid_levels_[index].limit_ordercount_ = 0;
}

void OrderBook::ResetAskLevel(int index)
{
    ask_levels_[index].limit_ordercount_ = 0;
}

/**
 * Rebuild/re-centre index when base_index_ moves past the upper limit
 * Shift all levels such that base_bid_index/base_ask_index (pointing to new_int_price_) is restored to
 * INITIAL_BASE_INDEX
 * Assign correct limit_int_price_ for the rest of the levels
 *
 */
void OrderBook::RebuildIndexHighAccess(char t_buysell_, int new_int_price_)
{
    switch (t_buysell_)
    {
    case 'B':
    {
        const int offset_ =
            new_int_price_ - bid_levels_[initial_tick_size_].limit_int_price_;

        int index_ = 0;
        for (; index_ + offset_ < (int)bid_levels_.size(); index_++)
        {
            bid_levels_[index_].limit_int_price_ =
                bid_levels_[index_ + offset_].limit_int_price_;
            bid_levels_[index_].limit_ordercount_ =
                bid_levels_[index_ + offset_].limit_ordercount_;
            bid_levels_[index_].limit_size_ =
                bid_levels_[index_ + offset_].limit_size_;
            bid_levels_[index_].limit_price_ =
                bid_levels_[index_ + offset_].limit_price_;
        }

        base_bid_index_ = initial_tick_size_;

        index_ = (int)bid_levels_.size() - offset_;
        if (index_ < 0)
        {
            index_ = 0;
        }

        for (; index_ < (int)bid_levels_.size(); index_++)
        {
            bid_levels_[index_].limit_int_price_ = new_int_price_ - (base_bid_index_ - index_);
            bid_levels_[index_].limit_ordercount_ = 0;
            bid_levels_[index_].limit_size_ = 0;
            bid_levels_[index_].limit_price_ =
                GetDoublePx(bid_levels_[index_].limit_int_price_);
        }
    }
    break;
    case 'S':
    {
        const int offset_ =
            ask_levels_[initial_tick_size_].limit_int_price_ - new_int_price_;

        int index_ = 0;
        for (; index_ + offset_ < (int)ask_levels_.size(); index_++)
        {
            ask_levels_[index_].limit_int_price_ =
                ask_levels_[index_ + offset_].limit_int_price_;
            ask_levels_[index_].limit_ordercount_ =
                ask_levels_[index_ + offset_].limit_ordercount_;
            ask_levels_[index_].limit_size_ =
                ask_levels_[index_ + offset_].limit_size_;
            ask_levels_[index_].limit_price_ =
                ask_levels_[index_ + offset_].limit_price_;
        }

        base_ask_index_ = initial_tick_size_;

        index_ = (int)ask_levels_.size() - offset_;
        if (index_ < 0)
        {
            index_ = 0;
        }

        for (; index_ < (int)ask_levels_.size(); index_++)
        {
            ask_levels_[index_].limit_int_price_ = new_int_price_ + (base_ask_index_ - index_);
            ask_levels_[index_].limit_ordercount_ = 0;
            ask_levels_[index_].limit_size_ = 0;
            ask_levels_[index_].limit_price_ =
                GetDoublePx(ask_levels_[index_].limit_int_price_);
        }
    }
    break;
    default:
        break;
    }
}

/**
 * Rebuild/re-centre index when base_index_ moves below the lower limit
 * Shift all levels such that base_bid_index/base_ask_index (pointing to new_int_price_) is restored to
 * INITIAL_BASE_INDEX
 * Assign correct limit_int_price_ for the rest of the levels
 */

void OrderBook::RebuildIndexLowAccess(char t_buysell_, int new_int_price_)
{
    switch (t_buysell_)
    {
    case 'B':
    {
        int offset_ = bid_levels_[initial_tick_size_].limit_int_price_ - new_int_price_;

        for (int index_ = bid_levels_.size() - 1; index_ >= offset_; index_--)
        {
            bid_levels_[index_].limit_int_price_ =
                bid_levels_[index_ - offset_].limit_int_price_;
            bid_levels_[index_].limit_ordercount_ =
                bid_levels_[index_ - offset_].limit_ordercount_;
            bid_levels_[index_].limit_size_ =
                bid_levels_[index_ - offset_].limit_size_;
            bid_levels_[index_].limit_price_ =
                bid_levels_[index_ - offset_].limit_price_;
        }

        base_bid_index_ = initial_tick_size_;

        // Offset can be quit huge, restrict size/price/ordercount resetting to the size of the array
        offset_ = std::min(offset_, (int)bid_levels_.size());

        for (int index_ = 0; index_ < offset_; index_++)
        {
            bid_levels_[index_].limit_int_price_ = new_int_price_ - (base_bid_index_ - index_);
            bid_levels_[index_].limit_ordercount_ = 0;
            bid_levels_[index_].limit_size_ = 0;
            bid_levels_[index_].limit_price_ =
                GetDoublePx(bid_levels_[index_].limit_int_price_);
        }
    }
    break;
    case 'S':
    {

        int offset_ = new_int_price_ - ask_levels_[initial_tick_size_].limit_int_price_;

        for (int index_ = ask_levels_.size() - 1; index_ >= offset_; index_--)
        {
            ask_levels_[index_].limit_int_price_ =
                ask_levels_[index_ - offset_].limit_int_price_;
            ask_levels_[index_].limit_ordercount_ =
                ask_levels_[index_ - offset_].limit_ordercount_;
            ask_levels_[index_].limit_size_ =
                ask_levels_[index_ - offset_].limit_size_;
            ask_levels_[index_].limit_price_ =
                ask_levels_[index_ - offset_].limit_price_;
        }

        base_ask_index_ = initial_tick_size_;

        // Offset can be quit huge, restrict size/price/ordercount resetting to the size of the array
        offset_ = std::min(offset_, (int)ask_levels_.size());

        for (int index_ = 0; index_ < offset_; index_++)
        {
            ask_levels_[index_].limit_int_price_ = new_int_price_ + (base_ask_index_ - index_);
            ask_levels_[index_].limit_ordercount_ = 0;
            ask_levels_[index_].limit_size_ = 0;
            ask_levels_[index_].limit_price_ =
                GetDoublePx(ask_levels_[index_].limit_int_price_);
        }
    }
    break;
    default:
        break;
    }
}

void OrderBook::BuildIndex(char t_buysell_, int int_price_)
{
    int int_bid_price_ = (t_buysell_ == 'B') ? int_price_ : int_price_ - 1;
    int int_ask_price_ = int_bid_price_ + 1;

    base_bid_index_ = initial_tick_size_;
    base_ask_index_ = initial_tick_size_;

    for (int index_ = 0; index_ < (int)bid_levels_.size(); index_++)
    {
        bid_levels_[index_].limit_int_price_ = int_bid_price_ - (base_bid_index_ - index_);
        ask_levels_[index_].limit_int_price_ = int_ask_price_ + (base_ask_index_ - index_);

        bid_levels_[index_].limit_ordercount_ = 0;
        ask_levels_[index_].limit_ordercount_ = 0;

        bid_levels_[index_].limit_size_ = 0;
        ask_levels_[index_].limit_size_ = 0;

        bid_levels_[index_].limit_price_ =
            GetDoublePx(bid_levels_[index_].limit_int_price_);
        ask_levels_[index_].limit_price_ =
            GetDoublePx(ask_levels_[index_].limit_int_price_);
    }

    initial_book_constructed_ = true;
}

void OrderBook::ResetBook()
{
    is_ready_ = false;
    initial_book_constructed_ = false;
}

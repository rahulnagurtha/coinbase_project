#include <vector>
#include <deque>
#include <string>
#include <cstdint>
#include <typeinfo>

#define LOW_ACCESS_INDEX 50
#define DEBUG_MODE_ON 0

struct PriceLevelInfo
{
    int limit_int_price_;  // integer value of the price, computed by dividing by min price increment.
    double limit_price_;   // price at this level
    int limit_size_;       // cumulative size at this level
    int limit_ordercount_; // cumulative count of orders at this level
};

struct OrderBook
{
    double min_price_increment_;

    std::string exchange_symbol_;

    std::vector<PriceLevelInfo> bid_levels_;
    std::vector<PriceLevelInfo> ask_levels_;

    bool is_ready_;
    bool initial_book_constructed_;

    unsigned int base_bid_index_;
    unsigned int base_ask_index_;

    unsigned int initial_tick_size_;
    unsigned int max_tick_range_;

    // functions
    OrderBook(std::string t_exchange_symbol_, double min_price_increment);

    ~OrderBook(){};

    void ResetBidLevel(int index);
    void ResetAskLevel(int index);

    void BuildIndex(char t_buysell_, int int_price_);

    void Initialize();

    void ResetBook();

    std::string ShowMarket();

    bool IsBidLevelEmpty(int bid_index);
    bool IsAskLevelEmpty(int ask_index);

    bool IsAskBookEmpty();
    bool IsBidBookEmpty();

    void UpdateBidLevel(int index, int size, int ordercount);
    void UpdateAskLevel(int index, int size, int ordercount);

    void RebuildIndexHighAccess(char t_buysell_, int new_int_price_);
    void RebuildIndexLowAccess(char t_buysell_, int new_int_price_);

    double min_price_increment() const
    {
        return min_price_increment_;
    }

    int GetBidIndex(int int_price)
    {
        return base_bid_index_ - (bid_levels_[base_bid_index_].limit_int_price_ - int_price);
    }

    int GetAskIndex(int int_price)
    {
        return base_ask_index_ + (ask_levels_[base_ask_index_].limit_int_price_ - int_price);
    }

    int GetBidIntPrice(int index) { return (index >= 0 ? bid_levels_[index].limit_int_price_ : 0); }

    int GetAskIntPrice(int index) { return (index >= 0 ? ask_levels_[index].limit_int_price_ : 0); }

    int GetBidSize(int index) { return (index >= 0 ? bid_levels_[index].limit_size_ : 0); }

    int GetAskSize(int index) { return (index >= 0 ? ask_levels_[index].limit_size_ : 0); }

    double GetBidPrice(int index) { return (index >= 0 ? bid_levels_[index].limit_price_ : 0); }

    double GetAskPrice(int index) { return (index >= 0 ? ask_levels_[index].limit_price_ : 0); }

    int GetBidOrders(int index) { return (index >= 0 ? bid_levels_[index].limit_ordercount_ : 0); }

    int GetAskOrders(int index) { return (index >= 0 ? ask_levels_[index].limit_ordercount_ : 0); }

    double GetDoublePx(const int t_int_price_) const { return min_price_increment_ * t_int_price_; }

    int GetIntPx(const double &t_price_) const { return t_price_ / min_price_increment_; }
};

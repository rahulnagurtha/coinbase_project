#include <vector>
#include <map>
#include <unordered_map>
#include <order_book.hpp>

// Order struct
struct OrderInfo
{
    double price;
    int size;
    uint8_t side;

    OrderInfo() : price(0.0), size(-1), side('-') {}
    OrderInfo(double t_price, int t_size, uint8_t t_side)
    {
        price = t_price;
        size = t_size;
        side = t_side;
    }
};

// This is the class which manipulates the underlying order book upon
// various events
class OrderBookManager
{
  private:
    std::unordered_map<uint64_t, OrderInfo>::iterator bid_order_info_map_iter_;
    std::unordered_map<uint64_t, OrderInfo>::iterator ask_order_info_map_iter_;

    // containers to hold all the live orders
    std::unordered_map<uint64_t, OrderInfo> order_id_to_bid_order_info_map_;
    std::unordered_map<uint64_t, OrderInfo> order_id_to_ask_order_info_map_;

    // The underlying order book
    OrderBook &order_book_;

  public:
    OrderBookManager(OrderBook &t_order_book);

    // Main Functions
    void OnOrderAdd(uint64_t t_order_id_, uint8_t t_side_, double t_price_, int t_size_);
    void OnOrderDelete(uint64_t t_order_id_, uint8_t t_side_);
    void OnOrderModify(uint64_t t_order_id_, uint8_t t_side_, int t_new_size_, uint64_t t_new_order_id_);
    void OnOrderReplace(uint64_t t_order_id_, uint8_t t_side_, double t_new_price_,
                        int t_new_size_, uint64_t t_new_order_id_);
    void OnOrderExec(uint64_t t_order_id_, uint8_t t_side_, double t_price_, int t_size_exec_);
    void OnOrderResetBegin();
    void OnOrderResetEnd();
    void UpdateBaseBidIndex();
    void UpdateBaseAskIndex();
    std::string ShowMarket() {
        return order_book_.ShowMarket();
    }
};

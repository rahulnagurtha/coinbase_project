#include "order_book_manager.hpp"
#include <iostream>

int main() {
    double min_price_increment;
    std::cout << "Enter min_price_increment of the product: ";
    std::cin >> min_price_increment;
    OrderBook ob("AMAZON", min_price_increment);
    OrderBookManager om(ob);

    while(1) {
        int option;
        std::cout << "Choose the order type : \n";
        std::cout << "1 -> Order Add\n";
        std::cout << "2 -> Order Delete\n";
        std::cout << "3 -> Order Modify\n";
        std::cout << "4 -> Order Replace\n";
        std::cout << "5 -> Order Exec\n";
        std::cout << "6 -> View Order Book\n";

        std::cin >> option;

        if(option == 1) {
            uint64_t t_order_id_;
            uint8_t t_side_;
            double t_price_;
            int t_size_;
            std::cout << "Enter Buy/Sell (B/S), order_id, order_price, order_size\n";
            std::cin >> t_side_ >> t_order_id_ >> t_price_ >> t_size_;
            om.OnOrderAdd(t_order_id_, t_side_, t_price_, t_size_);
        }
        else if(option == 2) {
            uint64_t t_order_id_;
            uint8_t t_side_;
            std::cout << "Enter Buy/Sell (B/S), order_id\n";
            std::cin >> t_side_ >> t_order_id_;
            om.OnOrderDelete(t_order_id_, t_side_);
        }
        else if(option == 3) {
            uint64_t t_order_id_;
            uint8_t t_side_;
            int t_new_size_;
            uint64_t t_new_order_id_;
            std::cout << "Enter Buy/Sell (B/S), order_id, new_size, new_order_id\n";
            std::cin >> t_side_ >> t_order_id_ >> t_new_size_ >> t_new_order_id_;
            om.OnOrderModify(t_order_id_, t_side_, t_new_size_, t_new_order_id_);
        }
        else if(option == 4) {
            uint64_t t_order_id_;
            uint8_t t_side_;
            double t_new_price_;
            int t_new_size_;
            uint64_t t_new_order_id_;
            std::cout << "Enter Buy/Sell (B/S), order_id, new_price, new_size, new_order_id\n";
            std::cin >> t_side_ >> t_order_id_ >> t_new_price_ >> t_new_size_ >> t_new_order_id_;
            om.OnOrderReplace(t_order_id_, t_side_, t_new_price_, t_new_size_, t_new_order_id_);
        }
        else if(option == 5) {
            uint64_t t_order_id_;
            uint8_t t_side_;
            double t_price_;
            int t_size_exec_;
            std::cout << "Enter Passive_side (B/S), order_id, exec_price, exec_size\n";
            std::cin >> t_side_ >> t_order_id_ >> t_price_ >> t_size_exec_;
            om.OnOrderExec(t_order_id_, t_side_, t_price_, t_size_exec_);
        }
        else {
            std::cout << om.ShowMarket() << std::endl;
        }
    }
    return 0;
}
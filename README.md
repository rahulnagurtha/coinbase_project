**Order_Book:**

The only data structure used in maintaining the status of the order book is STL Vector.


Order Book can be constructed in multiple ways. The simplest idea is to use 2 STL unordered_maps/hash_maps one for each bid, ask to store the info corresponding to each price level. The key could be price level and
the value could be a struct (total_size, num_of_orders_at_this_price_level).
Now apart from maintaining the state of each of the price level in STL unordered_maps/hash_maps, we also need to maintain the list of live orders on Bid/Ask side in a seperate STL unordered_maps/hash_maps where the key is order_id and the value is the meta-data of the order.


Now when the new order arrives, it has to be added to the map of live orders(Expected O(1) time) and
the corresponding price level in order book has to be updated with the new size and order count, this
would take extra expected O(1) time. And hence this leads to Expected time complexity of O(1) + O(1) time.

But we also need to keep in mind that search in a hash_map can take upto O(N) in worst case and hence in worst case the above algorithm might take O(N) + O(N) time. If we care about being very fast which is the case in HFT kind of setup, we would want to optimize this further. 

Now consider the scenario where the price levels are just integers, the easiest way would be to construct an order book using an array. For example if we declare an array of size 10^6, where each index could
correspond to respective price level, then to access nth level, it takes constant time ignoring the Cache hits/misses. On a normal day where the stock price of the product doesn't move much, we know that there is an upper and lower limit on the number of price levels on both bid/ask side. For example we can losely say that there will be not more than 500 levels on each bid/ask side. And now instead of declaring an array
of size 10^6 and maintain one-one mapping from index to price level, we can use transformation this array
and say that index-i corresponds to (base_price + i) price, for example we can say that ith index in the array corresponds to (100 + i) price. If we also keep track of the index of best bid/ask level (base_bid_index, base_ask_index), we can find out the index of the ith level in O(1) time and thereby access the information at that level, and since we know the index of the best bid/ask level in the array, given the price, we can easily say the index this price corresponds to (simple math).


This way any level in the order book can be accessed/updated in constant time.

So far the assumption was that prices are integral, which is not the case. To overcome this, we simply
divide the actual price with min_price_increment and this turns out to be an integer always(can be proved very easily). And once we convert the decimal price to integer price, all the above operations can be performed as discused in constant time.

One other issue in maintaining the order book in the form of array is the size of the array which is constant, now if we want to increase the size of the array during runtime, we can do that in O(size_of_book) time. Though this operation is very costly this doesn't happen very often, this can also be avoided by allocating suffienciently large amount of memory in the beginning. Or the other heuristic could be to ignore the incoming orders which would be very deeper in the order book.


**Order_Book_Manager:**

This is an interface to manipulate the underlying order book. Also this class maintains the list of live orders on both Bid/Ask Side in STL unoredered_map(hash_map) where the key is the OrderId and the value is the meta-data of the order.


Let's look at the time complexity of the different order types.

**OrderAdd:** This just involves adding it to the live order map which takes O(num_of_live_orders) in worst case, O(1) in best case and thereby modifying the corresponding bid/ask level in the underlying order book, which is O(1) as discussed above. So total time is O(1) in best case and O(num_of_live_orders) in worst case.

**OrderDelete:** This just involves deleting the order from live order map which takes O(num_of_live_orders) in worst case, O(1) in best case and thereby modifying the corresponding bid/ask level in the underlying order book, which is O(1) as discussed above. So total time is O(num_of_live_orders) in worst case, O(1) in best case.


**OrderModify:** This just involves modifying the order meta-data in live order map which takes O(num_of_live_orders) in worst case, O(1) in best case and thereby modifying the corresponding bid/ask level in the underlying order book, which is O(1) as discussed above. So total time is O(num_of_live_orders) in worst case, O(1) in best case


**OrderExec:** This just involves modifying the order meta-data in live order map which takes O(num_of_live_orders) in worst case, O(1) in best case and thereby modifying the corresponding bid/ask level in the underlying order book, which is O(1) as discussed above. So total time is O(num_of_live_orders) in worst case, O(1) in best case.


**ShowTop10Levels:** Since we know the index of the best bid/ask level in the array of the underlying order book, we have to traverse further until we see the next 10 levels on each bid/ask side. The time complexity here is the difference between the int_price_level (price divided by min_price_increment) of the best bid/ask level and the int_price_level of 10th bid/ask level. For example, if min_price_increment is 0.5 and the best bid price is 100 and the 10th best bid price is 90 it takes (100/0.5 - 90/0.5) == (20) simple operations. While traversing each of these 10 levels the VWAP bid/ask price can be computed accordingly.



**Note:** Instead of using the unordered_map, we can instead use the map STL and we can say that time complexity of search operation is O(log(N)) in worst case.
 

**Extra question in bonus section, "for each level, print the average/medium/min/max order sizes"**

Max/Min Order Size at each level: This can be easily found by maintaining the prioirity_queue for each of the levels and updating this data structure takes O(log(order_count_at_this_level)) and finding the max/min element in priroity_queue/heap takes O(1).

Medium/Median order size: This is equivalent to https://www.geeksforgeeks.org/median-of-stream-of-integers-running-integers/

Average: This can be computed easily as (total_orders_size/total_order_count).



**How to run a sample toy_program:**

Compile: g++ -std=c++11 -o order_manager test_program.cpp order_book.cpp order_book_manager.cpp
Run: ./order_manager

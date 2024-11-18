#include <iostream>
#include <map>
#include <list>
#include <chrono>
#include "./order.cpp"

using OrderPtr = std::shared_ptr<Order>;
using OrderQueue = std::list<OrderPtr>;

struct PriceLevelInfo
{
    Price price_;
    Quantity quantity_;
};

using PriceLevelInfos = std::unordered_map<Price, PriceLevelInfo>;
using PriceLevelLiquidity = std::unordered_map<Price, Quantity>;

struct Trade
{
    Price transactionPrice_;
    OrderId askOrderId_;
    OrderId bidOrderId_;
    Quantity matchedQuantity_;
};

class Orderbook
{
private:

    struct OrderEntry
    {
        OrderPtr order_;
        OrderQueue::iterator location_;
    };

    PriceLevelLiquidity bidPriceLevelLiquidity_;
    PriceLevelLiquidity askPriceLevelLiquidity_;
    std::map<Price, OrderQueue, std::greater<Price> > bids_;
    std::map<Price, OrderQueue, std::less<Price> > asks_;
    std::unordered_map<OrderId, OrderEntry> orderMap;

    //private method
    bool canMatch(Price askPrice, Price bidPrice)
    {
       return askPrice <= bidPrice; 
    }

    void match()
    {
        while (true)
        {
            if (bids_.empty() || asks_.empty())
                break;

            auto& [bidPrice, bidQueue] = *bids_.begin();
            auto& [askPrice, askQueue] = *asks_.begin();

            if (!canMatch(askPrice, bidPrice)) 
                break;
            
            Price transactionPrice = (askPrice + bidPrice) / 2.00;
            
            while (bidQueue.size() && askQueue.size())
            {
                auto start = std::chrono::high_resolution_clock::now();
                auto& bid = bidQueue.front(); 
                auto& ask = askQueue.front();

                Quantity quantity = std::min(bid->getRemainingQuantity(), ask->getRemainingQuantity());

                bid->fill(quantity);
                ask->fill(quantity);
                bidPriceLevelLiquidity_[bid->getPrice()] -= quantity;
                askPriceLevelLiquidity_[ask->getPrice()] -= quantity;
                std::cout << "Order Filled ;" << " Bid Order ID (" << bid->getOrderId() << ") Ask Order ID (" << ask->getOrderId() << ") Quantity (" << quantity << ") ";

                if (bid->isFilled())
                {
                    orderMap.erase(bid->getOrderId());
                    bidQueue.pop_front();
                }

                if (ask->isFilled())
                {
                    orderMap.erase(ask->getOrderId());
                    askQueue.pop_front();
                }
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
                std::cout << "Time Used : " << duration.count() << " nanoseconds\n";
            }

            if (bidQueue.empty())
            {
                bids_.erase(bidPrice);
            }
            if (askQueue.empty())
            {
                asks_.erase(askPrice);
            }
        }
    }
public:

    void addOrder(OrderType orderType, Price price, Side side, Quantity quantity)
    {
        OrderPtr order = std::make_shared<Order>(orderType, price, side, quantity);
        OrderQueue::iterator iterator;

        if (order->getSide() == Side::Bid)
        {
            auto &orderQueue = bids_[order->getPrice()];
            orderQueue.push_back(order);
            iterator = next(orderQueue.begin(), orderQueue.size() - 1);
            bidPriceLevelLiquidity_[price] += quantity;
        }
        else
        {
            auto &orderQueue = asks_[order->getPrice()];
            orderQueue.push_back(order);
            iterator = next(orderQueue.begin(), orderQueue.size() - 1);
            askPriceLevelLiquidity_[price] += quantity;
        }
        OrderEntry orderEntry = { order, iterator }; 
        orderMap.insert(std::make_pair(order->getOrderId(), orderEntry));

        return match();
    }

    void cancelOrder(OrderId orderId)
    {
        OrderEntry orderEntry = orderMap.find(orderId)->second;
        OrderPtr order = orderEntry.order_; 
        OrderQueue::iterator iterator = orderEntry.location_;
        orderMap.erase(orderId);

        if (order->getSide() == Side::Bid)
        {
            bids_[order->getPrice()].erase(iterator);
        }
        else
        {
            asks_[order->getPrice()].erase(iterator);
        }
    }

    void print()
    {
        std::cout << "=== Order Book ===" << "\n";
        for (auto it = asks_.rbegin() ; it != asks_.rend() ; it++)
        {
            for (auto &order : it->second)
            {
                std::cout << "Order Id : " << order->getOrderId() << " Price : " << order->getPrice() << " Quantity : " << order->getRemainingQuantity() << "\n";
            }
        }
        std::cout << "==================" << "\n";
        for (auto it = bids_.begin() ; it != bids_.end() ; it++)
        {
            for (auto &order : it->second)
            {
                std::cout << "Order Id : " << order->getOrderId() << " Price : " << order->getPrice() << " Quantity : " << order->getRemainingQuantity() << "\n";
            }
        }
    }

    void printOrderBook()
    {
        std::cout << "=== Simple Orderbook ===" << "\n";
        for (auto it = asks_.rbegin() ; it != asks_.rend() ; it++)
        {
            std::cout << "Price : " << it->first << " QTY : " << askPriceLevelLiquidity_[it->first] << "\n";
        }
        std::cout << "========================" << "\n";
        for (auto it = bids_.begin() ; it != bids_.end() ; it++)
        {
            std::cout << "Price : " << it->first << " QTY : " << bidPriceLevelLiquidity_[it->first] << "\n";
        }
    }
};
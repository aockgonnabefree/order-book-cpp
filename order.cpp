using Price = double;
using Quantity = int;
using OrderId = int;

enum Side {
    Bid,
    Ask
};
enum OrderType
{
    Market,
    Limit
};

class Order
{
private:
    OrderType orderType_;
    Price price_;
    Side side_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
    OrderId orderId_;
public:
    Order(OrderType orderType, Price price, Side side, Quantity quantity)
        : orderType_( orderType ), price_( price ), side_( side ), initialQuantity_( quantity ) , remainingQuantity_( quantity ) { }
    Order(OrderType orderType, Price price, Side side, Quantity quantity, OrderId orderId)
        : orderType_( orderType ), price_( price ), side_( side ), initialQuantity_( quantity ) , remainingQuantity_( quantity ), orderId_( orderId ) { }
    
    //getter
    OrderType getOrderType() const { return orderType_ ; }
    Price getPrice() const { return price_; }
    Side getSide() const { return side_;}
    Quantity getInitialQuantity() const { return initialQuantity_;}
    Quantity getRemainingQuantity() const { return remainingQuantity_;}
    Quantity getFilledQuantity() const { return initialQuantity_ - remainingQuantity_;}
    OrderId getOrderId() const { return orderId_; }

    //method
    void fill(Quantity quantity)
    {
        if (quantity > remainingQuantity_)
        {
            return ;
        }
        remainingQuantity_ -= quantity;
    }

    bool isFilled() const { return remainingQuantity_ == 0; }
};

#include <iostream>
#include "task.h"

int main() {
    int checkouts_count = 3;
    double client_intensity = 40;
    double goods_intensity = 20;
    double avg_goods = 5;
    int max_queue_length = 10;
    double generating_time = 3.8;

    Shop shop(checkouts_count, client_intensity, goods_intensity, avg_goods, max_queue_length);
    shop.work(generating_time);

    auto stats = shop.getShopStats();
    auto theory_indicators = Indicators::getTheoryIndicators(goods_intensity, avg_goods, client_intensity, checkouts_count, max_queue_length);
    auto real_indicators = Indicators::getRealIndicators(stats);

    std::cout << stats.toString() << std::endl;
    std::cout << "Theory " << theory_indicators.toString();
    std::cout << "Real " << real_indicators.toString();
}
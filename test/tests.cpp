// Copyright 2021 GHA Test Team
#include <gtest/gtest.h>
#include "task.h"

TEST(InputCheck, shopIncorrectInput) {
    EXPECT_ANY_THROW(Shop(1, 2, 3, 4, -5));
}

TEST(InputCheck, shopCorrectInput) {
    EXPECT_NO_THROW(Shop(1, 2, 3, 4, 5));
}

TEST(InputCheck, clientIncorrectInput) {
    EXPECT_ANY_THROW(Client(-5));
}

TEST(InputCheck, clientCorrectInput) {
    EXPECT_NO_THROW(Client(5));
}

TEST(InputCheck, indicatorsIncorrectInput) {
    EXPECT_ANY_THROW(Indicators(2, 4, 5));
}

TEST(InputCheck, indicatorsCorrectInput) {
    EXPECT_NO_THROW(Indicators(0.5, 0.5, 5));
}

TEST(WorkCheck, checkShopStats) {
    int checkouts_count = 3;
    double client_intensity = 70;
    double goods_intensity = 20;
    double avg_goods = 3;
    int max_queue_length = 10;
    double generating_time = 1;

    Shop shop(checkouts_count, client_intensity, goods_intensity, avg_goods, max_queue_length);
    EXPECT_ANY_THROW(shop.work(-1));

    shop.work(generating_time);
    auto stats = shop.getShopStats();

    auto eps = 0.1;
    EXPECT_TRUE(stats.served_clients < stats.unserved_clients);
    EXPECT_TRUE(stats.total_time >= generating_time);
    EXPECT_TRUE(stats.avg_checkout_worktime > stats.avg_checkout_idletime);
    EXPECT_TRUE(stats.avg_queue_length <= max_queue_length);
    EXPECT_TRUE(abs(stats.avg_time_in_checkout - avg_goods / goods_intensity) <= eps);
}

TEST(WorkCheck, checkTheoryIndicators) {
    int checkouts_count = 3;
    double client_intensity = 40;
    double goods_intensity = 20;
    double avg_goods = 5;
    int max_queue_length = 10;

    auto theory_indicators = Indicators::getTheoryIndicators(goods_intensity, avg_goods, client_intensity, checkouts_count, max_queue_length);
    auto eps = 0.001;

    EXPECT_TRUE(abs(theory_indicators.failure_probability - 0.7) < eps);
    EXPECT_TRUE(abs(theory_indicators.relative_throughput - 0.3) < eps);
    EXPECT_TRUE(abs(theory_indicators.absolute_throughput - 60) < eps);
}

TEST(WorkCheck, checkRealIndicators) {
    int checkouts_count = 3;
    double client_intensity = 40;
    double goods_intensity = 20;
    double avg_goods = 5;
    int max_queue_length = 10;
    double generating_time = 3;

    Shop shop(checkouts_count, client_intensity, goods_intensity, avg_goods, max_queue_length);
    shop.work(generating_time);

    auto stats = shop.getShopStats();

    auto real_indicators = Indicators::getRealIndicators(stats);

    EXPECT_TRUE(real_indicators.failure_probability >= real_indicators.relative_throughput);
}

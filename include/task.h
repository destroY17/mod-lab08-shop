#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>

typedef std::unique_lock<std::mutex> lock;
typedef std::chrono::steady_clock watch;
typedef std::chrono::duration<double> doubleTime;

class ShopStats {
public:
    double total_time = 0;
    int served_clients = 0;
    int unserved_clients = 0;
    double avg_queue_length = 0;
    double avg_time_in_queue = 0;
    double avg_time_in_checkout = 0;
    double avg_checkout_worktime = 0;
    double avg_checkout_idletime = 0;
public:
    std::string toString();
};

class Indicators {
public:
    const double failure_probability;
    const double relative_throughput;
    const double absolute_throughput;
private:
    static int factorial(int n);
    static double getFreeProbability(double intensity, int checkouts_count, int max_queue_length);
public:
    explicit Indicators(double failure_probability, double relative, double absolute);
    std::string toString();
    static Indicators getTheoryIndicators(double goods_intensity, int avg_goods, double customer_intensity,
        int checkouts_count, int max_queue_length);
    static Indicators getRealIndicators(ShopStats stats);
};

class Client {
public:
    const int goods_count;
    const watch::time_point start_queue_time;
public:
    explicit Client(int goods_count);
};

class Shop {
private:
    bool is_working;

    int checkouts_count;
    double client_intensity;
    double goods_intensity;
    double avg_goods;
    int max_queue_length;

    double total_checkout_time;
    double total_queue_time;
    int total_queue_length;
    int queue_measurements;

    ShopStats stats;
    std::vector<std::thread> checkouts;
    std::queue<Client> clients_queue;

    std::mutex mtx;
private:
    void clear_work_data();
    void checkoutWork();
    void generateClients(double generating_time);
public:
    explicit Shop(int checkouts_count, double client_intensity, double goods_intensity,
        double avg_goods, int max_queue_length);
    void work(double generating_time);
    ShopStats getShopStats();
};
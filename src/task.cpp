#include "task.h"
#include <string>
#include <sstream>
#include <cmath>
#include <cstdlib>

std::string ShopStats::toString() {
    std::stringstream ss;
    ss << "Shop Statistics:\n";
    ss << "Total time: " << total_time << " seconds\n";
    ss << "Served Clients: " << served_clients << "\n";
    ss << "Unserved Clients: " << unserved_clients << "\n";
    ss << "Average queue length: " << avg_queue_length << "\n";
    ss << "Average Time in Queue: " << avg_time_in_queue << " seconds\n";
    ss << "Average Time in Checkout: " << avg_time_in_checkout << " seconds\n";
    ss << "Average Checkout Work Time: " << avg_checkout_worktime << " seconds\n";
    ss << "Average Checkout Idle Time: " << avg_checkout_idletime << " seconds\n";

    return ss.str();
}

Indicators::Indicators(double failure_probability, double relative, double absolute) :
    failure_probability{ failure_probability },
    relative_throughput{ relative },
    absolute_throughput{ absolute } {
    if (failure_probability < 0 || failure_probability > 1 ||
        relative < 0 || relative > 1 || absolute < 0) {
        throw std::invalid_argument("Incorrect parameters");
    }
}

std::string Indicators::toString() {
    std::stringstream ss;
    ss << "Indicators:\n";
    ss << "Failure Probability: " << failure_probability << "\n";
    ss << "Relative Throughput: " << relative_throughput << "\n";
    ss << "Absolute Throughput: " << absolute_throughput << "\n";

    return ss.str();
}

int Indicators::factorial(int n) {
    if (n == 0 || n == 1) {
        return 1;
    }
    else {
        return n * factorial(n - 1);
    }
}

double Indicators::getFreeProbability(double intensity, int checkouts_count, int max_queue_length) {
    double free_probability = 0;

    for (int i = 0; i <= checkouts_count; i++) {
        free_probability += pow(intensity, i) / factorial(i);
    }
    for (int i = 1; i <= max_queue_length; i++) {
        free_probability += pow(intensity, checkouts_count + i) / (factorial(checkouts_count) * pow(checkouts_count, i));
    }
    return pow(free_probability, -1);
}

Indicators Indicators::getTheoryIndicators(double goods_intensity, int avg_goods, double client_intensity,
    int checkouts_count, int max_queue_length) {
    auto intensity = client_intensity * avg_goods / goods_intensity;
    auto free_probability = getFreeProbability(intensity, checkouts_count, max_queue_length);
    auto failure_probability = pow(intensity, checkouts_count + max_queue_length) /
        (pow(checkouts_count, max_queue_length) * factorial(checkouts_count)) * free_probability;
    auto relative = 1 - failure_probability;
    auto absolute = client_intensity * avg_goods * relative;

    return Indicators(failure_probability, relative, absolute);
}

Indicators Indicators::getRealIndicators(ShopStats stats) {
    auto allClients = stats.served_clients + stats.unserved_clients;
    auto failure_probability = (double)stats.unserved_clients / allClients;
    auto relative = 1 - failure_probability;
    auto absolute = (double)stats.served_clients / stats.total_time;

    return Indicators(failure_probability, relative, absolute);
}

Client::Client(int goods_Count) :
    goods_count{ goods_Count },
    start_queue_time{ watch::now() } {
    if (goods_Count < 0)
        throw std::invalid_argument("goods_count will be >= 0");
}

Shop::Shop(int checkouts_count, double client_intensity, double goods_intensity,
    double avg_goods, int max_queue_length) :
    checkouts_count{ checkouts_count },
    client_intensity{ client_intensity },
    goods_intensity{ goods_intensity },
    avg_goods{ avg_goods },
    max_queue_length{ max_queue_length },
    checkouts{ std::vector<std::thread>() },
    clients_queue{ std::queue<Client>() },
    is_working{ false } {
    if (checkouts_count < 0 || client_intensity < 0 ||
        goods_intensity < 0 || avg_goods < 0 || max_queue_length < 0)
        throw std::invalid_argument("Invalid parameters");
    clear_work_data();
}

void Shop::clear_work_data() {
    stats = ShopStats();
    total_checkout_time = 0;
    total_queue_time = 0;
    total_queue_length = 0;
    queue_measurements = 0;
}

void Shop::work(double generating_time) {
    if (generating_time < 0)
        throw std::invalid_argument("generating_time will be >= 0");

    clear_work_data();
    auto begin_time = watch::now();

    checkouts.clear();
    for (int i = 0; i < checkouts_count; i++) {
        checkouts.emplace_back(&Shop::checkoutWork, this);
    }

    generateClients(generating_time);

    for (auto& checkout : checkouts)
    {
        checkout.join();
    }

    auto end_time = watch::now();

    stats.total_time = doubleTime(end_time - begin_time).count();
    stats.avg_checkout_worktime = total_checkout_time / checkouts_count;
    stats.avg_checkout_idletime = stats.total_time - stats.avg_checkout_worktime;
    stats.avg_time_in_queue = total_queue_time / stats.served_clients;
    stats.avg_time_in_checkout = total_checkout_time / stats.served_clients;
    stats.avg_queue_length = (double)total_queue_length / queue_measurements;
}

void Shop::checkoutWork() {
    while (is_working) {
        lock locker(mtx);

        if (clients_queue.empty()) {
            continue;
        }

        auto client = clients_queue.front();
        clients_queue.pop();
        locker.unlock();

        total_queue_time += doubleTime(watch::now() - client.start_queue_time).count();

        auto work_time = client.goods_count / goods_intensity;
        total_checkout_time += work_time;

        std::this_thread::sleep_for(doubleTime(work_time));

        stats.served_clients++;
    }
}

void Shop::generateClients(double generating_time) {
    auto begin_generate_time = watch::now();
    double different = 0;

    is_working = true;
    while (different < generating_time) {
        std::this_thread::sleep_for(doubleTime(1 / client_intensity));

        if (clients_queue.size() >= max_queue_length) {
            stats.unserved_clients++;
        }
        else {
            auto goods_count = 1 + std::rand() % (2 * static_cast<int>(std::round(avg_goods)) - 1);
            Client client(goods_count);
            clients_queue.push(client);
        }

        total_queue_length += clients_queue.size();
        queue_measurements++;

        different = doubleTime(watch::now() - begin_generate_time).count();
    }
    is_working = false;
}

ShopStats Shop::getShopStats() {
    return stats;
}
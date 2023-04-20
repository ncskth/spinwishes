#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void printHello(int n) {
    while (true) {
        cout << "hello" << endl;
        this_thread::sleep_for(chrono::seconds(n));
    }
}

void printLittle(int n) {
    while (true) {
        cout << "little" << endl;
        this_thread::sleep_for(chrono::seconds(n));
    }
}

void printWorld(int n) {
    while (true) {
        cout << "world" << endl;
        this_thread::sleep_for(chrono::seconds(n));
    }
}

int main() {
    thread t1(printHello, 1);
    thread t2(printLittle, 2);
    thread t3(printWorld, 3);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}

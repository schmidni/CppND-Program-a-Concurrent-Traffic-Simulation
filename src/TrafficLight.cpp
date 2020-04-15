#include "TrafficLight.h"
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
    // create lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); });

    // dequeue
    T msg = std::move(_queue.back());
    // _queue.pop_back(); // can't use pop_back since the outer intersections don't get a lot of traffic and the queue fills up
    _queue.clear();

    // return object
    return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
    std::lock_guard<std::mutex> gLock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight() { 
    _currentPhase = TrafficLightPhase::red; 
    _type = ObjectType::objectTrafficLight;
}

void TrafficLight::waitForGreen() {
    while (true) {
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
    // launch Phase cycling processing in a thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
    // Random number generator
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(4.0, 6.0);
    float cycleTime = dist(mt);

    // init variables for measuring time
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;

    while (true) {
        // sleep for 100 miliseconds is enough since we wait for 4-6s
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed = std::chrono::high_resolution_clock::now() - start;

        if (elapsed.count() > cycleTime) {
            // reset start time and get new cycle time
            start = std::chrono::high_resolution_clock::now();
            cycleTime = dist(mt);
            // toggle phase
            _currentPhase =
                (_currentPhase == TrafficLightPhase::green) ? red : green;

            // send update method to message queue using move semantics.
            TrafficLightPhase p = _currentPhase;
            _messageQueue.send(std::move(p));
        }
    }
}

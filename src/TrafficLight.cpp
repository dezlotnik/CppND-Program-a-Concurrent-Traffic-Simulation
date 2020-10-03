#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <random>

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mtx);
    _condition.wait(lck,[this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mtx);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {

        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (_queue.receive() == TrafficLightPhase::green) {
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    // set random duration between 4 and 6 seconds
    // https://stackoverflow.com/questions/2704521/generate-random-double-numbers-in-c
    // https://stackoverflow.com/questions/15461140/stddefault-random-engine-generate-values-between-0-0-and-1-0
    std::uniform_real_distribution<double> unif(4.0,6.0);
    std::random_device rd;
    std::default_random_engine re(rd());
    double cycle_duration = unif(re); // cycle duration in seconds
    double time_since_last_update = 0.0;

    std::chrono::time_point<std::chrono::system_clock> last_update;
    last_update = std::chrono::system_clock::now();
    while (true) {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto elapsed = std::chrono::system_clock::now() - last_update;
        time_since_last_update = std::chrono::duration<double>(elapsed).count();
        if (time_since_last_update >= cycle_duration) {
            auto new_phase = (isGreen()) ?  TrafficLightPhase::red : TrafficLightPhase::green;
            setCurrentPhase(new_phase);
            _queue.send(std::move(new_phase));
            last_update = std::chrono::system_clock::now();
        }
    }
}


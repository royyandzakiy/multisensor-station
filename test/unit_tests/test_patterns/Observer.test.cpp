#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <Observer.hpp> // Include the header containing Observable and Observer

/**
 * TEST CASES
 * ObserverTest
 * - should_receive_valid_weak_ptr_when_notified
 * - should_receive_expired_weak_ptr_when_observable_is_destroyed
 * 
 * ObservableTets
 * - should_notify_observer_when_notifyObservers_is_called
 * - should_notify_all_observers_when_notifyObservers_is_called
 * - should_not_notify_any_observer_when_no_observers_are_added
 * - should_not_notify_observer_when_it_has_been_removed
 */

class MockObserver : public Observer {
public:
    MOCK_METHOD(void, notify, (std::weak_ptr<Observable> observable), (override));
};

class ObserverTest : public::testing::Test {
protected:
    void SetUp() override {
        observable = std::make_shared<Observable>();
        mockObserver1 = std::make_shared<MockObserver>();
        mockObserver2 = std::make_shared<MockObserver>();
    }

    std::shared_ptr<Observable> observable;
    std::shared_ptr<MockObserver> mockObserver1;
    std::shared_ptr<MockObserver> mockObserver2;
};

// Observer
TEST_F(ObserverTest, should_receive_valid_weak_ptr_when_notified) {
    // obervable adds mockObserver1
    // obervable calls notifyObservers > mockObserver1.notify
    // check through mock method if observable being passed is a weak_ptr type
    observable->addObserver(mockObserver1);
    EXPECT_CALL(*mockObserver1, notify(::testing::_)).Times(1);
    observable->notifyObservers();
}

TEST_F(ObserverTest, should_receive_expired_weak_ptr_when_observable_is_destroyed) {
    // observable adds mockObserver1
    // observable is destroyed
    // check through mock method if observable being passed is a destroyed weak_ptr type
}

// Observable
TEST_F(ObserverTest, should_notify_observer_when_notifyObservers_is_called) {
    // observable adds mockObserver1, mockObserver2
    // obervable calls notifyObservers
    // check through mock method if notify has been called
}

TEST_F(ObserverTest, should_notify_all_observers_when_notifyObservers_is_called) {
    // observable adds mockObserver1, mockObserver2
    // obervable calls notifyObservers
    // check through mock method if mockObserver1.notify & mockObserver1.notify has been called
}

TEST_F(ObserverTest, should_not_notify_any_observer_when_no_observers_are_added) {
    // ???
}

TEST_F(ObserverTest, should_not_notify_observer_when_it_has_been_removed) {
    // observable adds mockObserver1, mockObserver2
    // mockObserver1 is somehow removed from observable
    // obervable calls notifyObservers
    // check through mock method if mockObserver1.notify has NOT been called
}
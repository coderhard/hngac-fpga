#include <chrono>
#include <string>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class BadActorNode : public rclcpp::Node {
public:
    BadActorNode() : Node("bad_actor") {
        publisher_ = this->create_publisher<std_msgs::msg::String>("cmd_untrusted", 10);
        
        // Timer: Publish a command every 500ms (2Hz)
        timer_ = this->create_wall_timer(
            500ms, std::bind(&BadActorNode::timer_callback, this));
            
        RCLCPP_INFO(this->get_logger(), "Bad Actor Node Started. Injecting faults...");
    }

private:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int counter_ = 0;

    void timer_callback() {
        auto message = std_msgs::msg::String();
        
        // LOGIC: Alternate between Subject 1 (Authorized) and Subject 99 (Unauthorized)
        // In the Gatekeeper, we only whitelisted Subject 1.
        
        int subject_id = (counter_ % 2 == 0) ? 1 : 99; 
        
        message.data = std::to_string(subject_id);
        
        if (subject_id == 1) {
             RCLCPP_INFO(this->get_logger(), "Sending [VALID] Command (Subject %d)", subject_id);
        } else {
             RCLCPP_INFO(this->get_logger(), "Sending [MALICIOUS] Command (Subject %d)", subject_id);
        }

        publisher_->publish(message);
        counter_++;
    }
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BadActorNode>());
    rclcpp::shutdown();
    return 0;
}

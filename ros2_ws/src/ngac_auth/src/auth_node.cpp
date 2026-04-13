#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <bitset>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "geometry_msgs/msg/twist.hpp"

constexpr size_t MAX_NODES = 256;
struct Hyperedge {
    std::bitset<MAX_NODES> subjects;
    std::bitset<MAX_NODES> objects;
    std::bitset<MAX_NODES> attributes;
};

class NGACEngine {
private:
    std::vector<Hyperedge> policy;
public:
    void add_rule(const Hyperedge& edge) { policy.push_back(edge); }
    bool authorize(size_t subject, size_t object, const std::bitset<MAX_NODES>& required_attrs) {
        for (const auto& edge : policy) {
            if (edge.subjects.test(subject) && edge.objects.test(object) && (required_attrs & edge.attributes) == required_attrs) return true;
        }
        return false;
    }
};

class AuthNode : public rclcpp::Node {
public:
    AuthNode() : Node("ngac_gatekeeper") {
        setup_policy();
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "cmd_untrusted", 10, std::bind(&AuthNode::topic_callback, this, std::placeholders::_1));
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        RCLCPP_INFO(this->get_logger(), "NGAC Gatekeeper: Driving /cmd_vel based on Auth");
    }

private:
    NGACEngine engine_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;

    void setup_policy() {
        Hyperedge edge;
        edge.subjects.set(1); 
        edge.objects.set(2);
        edge.attributes.set(5); 
        engine_.add_rule(edge);
    }

    void topic_callback(const std_msgs::msg::String::SharedPtr msg) const {
        int subject_id = -1;
        try {
            size_t pos = 0;
            subject_id = std::stoi(msg->data, &pos);
            if (pos != msg->data.size()) throw std::invalid_argument("partial parse");
        } catch (const std::exception&) {
            RCLCPP_WARN(this->get_logger(), "[BLOCK] Invalid subject payload: '%s'", msg->data.c_str());
            return;
        }
        if (subject_id < 0 || static_cast<size_t>(subject_id) >= MAX_NODES) {
            RCLCPP_WARN(this->get_logger(), "[BLOCK] Subject ID out of range: %d", subject_id);
            return;
        }
        std::bitset<MAX_NODES> required_attrs;
        required_attrs.set(5);

        // 1. Start Stopwatch
        auto start = std::chrono::high_resolution_clock::now();
        
        // 2. Run Logic
        bool authorized = const_cast<NGACEngine&>(engine_).authorize(subject_id, 2, required_attrs);
        
        // 3. Stop Stopwatch
        auto end = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        auto vel_msg = geometry_msgs::msg::Twist();
        if (authorized) {
            vel_msg.linear.x = 2.0;
            publisher_->publish(vel_msg);
            // CRITICAL: This log format is what the Python script looks for!
            RCLCPP_INFO(this->get_logger(), "[PASS] Time: %ld ns | Cmd forwarded", ns);
        } else {
            vel_msg.linear.x = 0.0;
            publisher_->publish(vel_msg);
            RCLCPP_WARN(this->get_logger(), "[BLOCK] Time: %ld ns | Unauthorized Subject %d", ns, subject_id);
        }
    }
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AuthNode>());
    rclcpp::shutdown();
    return 0;
}

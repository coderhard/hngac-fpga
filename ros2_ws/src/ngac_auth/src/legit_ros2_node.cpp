#include <chrono>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

// Legitimate ROS2 node with correct credentials.
//
// Subject 1, provenance = authenticated_ros2_node (1). This is what the 5D
// policy permits. Its commands pass all three dimensions (3D, 4D, 5D).
// Used in the adversarial demo alongside compromised_ros2_node to confirm
// that 5D enforcement produces zero false positives for legitimate traffic.
//
// Message format: "subject_id:provenance_mask"
//   subject_id = 1   (authorized subject)
//   provenance = 1   (authenticated_ros2_node, ProvenanceBit::authenticated_ros2_node = bit 0 = 1)

using namespace std::chrono_literals;

class LegitRos2Node : public rclcpp::Node {
public:
    explicit LegitRos2Node()
        : Node("legit_ros2_node") {
        publisher_ = create_publisher<std_msgs::msg::String>("cmd_untrusted", 10);

        const double hz = declare_parameter<double>("rate_hz", 2.0);
        const auto period_ms = std::chrono::milliseconds(
            static_cast<int>(1000.0 / hz));

        timer_ = create_wall_timer(
            period_ms,
            std::bind(&LegitRos2Node::send_command, this));

        RCLCPP_INFO(get_logger(),
            "Legitimate node online. Subject=1, Provenance=authenticated_ros2_node(1). "
            "Rate=%.1f Hz. All commands should PASS 5D.", hz);
    }

private:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::size_t count_ = 0;

    void send_command() {
        ++count_;
        std_msgs::msg::String msg{};
        // Subject 1, provenance = authenticated_ros2_node (1)
        msg.data = "1:1";
        publisher_->publish(msg);
        RCLCPP_INFO(get_logger(),
            "[LEGIT #%zu] subject=1 prov=authenticated_ros2_node(1)",
            count_);
    }
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LegitRos2Node>());
    rclcpp::shutdown();
    return 0;
}

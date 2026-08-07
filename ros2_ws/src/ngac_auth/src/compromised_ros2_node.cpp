#include <chrono>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

// Simulates a compromised authenticated ROS2 node.
//
// This node holds Subject 1 credentials — the same subject ID that the policy
// authorizes. Under 3D and 4D authorization, its commands pass because the
// subject, object, and attribute checks succeed and state is a wildcard.
// Under 5D authorization, commands are denied because the source provenance
// (remote_operator = 4) does not match the policy's required provenance
// (authenticated_ros2_node = 1).
//
// This is the command-provenance-abuse class in the paper: injection via a compromised
// authenticated node. SROS2 transport-layer identity checks pass (the node
// is Subject 1). The 5D application-layer check catches the wrong source type.
//
// Message format: "subject_id:provenance_mask"
//   subject_id = 1      (authorized subject — same as legitimate node)
//   provenance = 4      (remote_operator, ProvenanceBit::remote_operator = bit 2 = 4)

using namespace std::chrono_literals;

class CompromisedRos2Node : public rclcpp::Node {
public:
    explicit CompromisedRos2Node()
        : Node("compromised_ros2_node") {
        publisher_ = create_publisher<std_msgs::msg::String>("cmd_untrusted", 10);

        const double hz = declare_parameter<double>("rate_hz", 10.0);
        const auto period_ms = std::chrono::milliseconds(
            static_cast<int>(1000.0 / hz));

        timer_ = create_wall_timer(
            period_ms,
            std::bind(&CompromisedRos2Node::send_injection, this));

        RCLCPP_WARN(get_logger(),
            "Compromised node online. Subject=1 (authorized ID), "
            "Provenance=remote_operator(4). Rate=%.1f Hz. "
            "This node SHOULD pass 4D and be BLOCKED by 5D.",
            hz);
    }

private:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::size_t count_ = 0;

    void send_injection() {
        ++count_;
        std_msgs::msg::String msg{};
        // Subject 1, provenance = remote_operator (4)
        msg.data = "1:4";
        publisher_->publish(msg);
        RCLCPP_WARN(get_logger(),
            "[INJECT #%zu] Sending subject=1 prov=remote_operator(4) — provenance abuse",
            count_);
    }
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CompromisedRos2Node>());
    rclcpp::shutdown();
    return 0;
}

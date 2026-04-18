#include <chrono>
#include <memory>
#include <string>

#include "geometry_msgs/msg/twist.hpp"
#include "hngac_kernel.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

// Message format on cmd_untrusted: "subject_id:provenance_mask"
// provenance_mask is a decimal integer (bitmask over ProvenanceBit values).
// If no colon is present, provenance_mask defaults to 0.
// ProvenanceBit: 0=authenticated_ros2_node (1), 1=local_terminal (2), 2=remote_operator (4)
//
// Policy enforced:
//   subject=1, object=2, attr=5, required_states=0 (wildcard), required_provenance=1
//   (only authenticated_ros2_node is permitted for this command)
//
// This node demonstrates Attack Class 2: a node with valid Subject 1 credentials
// but wrong provenance type (e.g., remote_operator=4) is blocked by 5D but would
// pass 3D and 4D.

namespace {

using hngac::fpga::AuthorizationRequest;
using hngac::fpga::PolicyRule;
using hngac::fpga::ProvenanceBit;
using hngac::fpga::set_bit;
using hngac::fpga::set_provenance_bit;
using hngac::fpga::hngac_authorize;

constexpr std::uint16_t kSubject = 1;
constexpr std::uint16_t kObject  = 2;
constexpr std::uint16_t kAttr    = 5;

PolicyRule build_policy() {
    PolicyRule rule{};
    set_bit(rule.subjects, kSubject);
    set_bit(rule.objects,  kObject);
    set_bit(rule.attributes, kAttr);
    // required_states = 0: any system state is permitted
    set_provenance_bit(rule.required_provenance, ProvenanceBit::authenticated_ros2_node);
    return rule;
}

bool parse_message(const std::string& data,
                   std::uint16_t& subject_id,
                   std::uint32_t& provenance_mask) {
    try {
        const auto colon = data.find(':');
        const unsigned long raw_subject = std::stoul(
            colon == std::string::npos ? data : data.substr(0, colon));
        const unsigned long raw_prov = colon == std::string::npos
            ? 0UL
            : std::stoul(data.substr(colon + 1));
        if (raw_subject > 255 || raw_prov > 0xFF) {
            return false;
        }
        subject_id      = static_cast<std::uint16_t>(raw_subject);
        provenance_mask = static_cast<std::uint32_t>(raw_prov);
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace

class Gatekeeper5DNode : public rclcpp::Node {
public:
    Gatekeeper5DNode()
        : Node("ngac_gatekeeper_5d") {
        policy_ = build_policy();

        subscription_ = create_subscription<std_msgs::msg::String>(
            "cmd_untrusted", 10,
            std::bind(&Gatekeeper5DNode::on_command, this, std::placeholders::_1));
        publisher_ = create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

        RCLCPP_INFO(get_logger(),
            "5D Gatekeeper online. Policy: subject=%u object=%u attr=%u "
            "provenance=authenticated_ros2_node(1) state=wildcard",
            kSubject, kObject, kAttr);
    }

    ~Gatekeeper5DNode() {
        RCLCPP_INFO(get_logger(),
            "--- Session summary ---");
        RCLCPP_INFO(get_logger(),
            "Total received: %zu | Passed: %zu | Blocked-subject: %zu | Blocked-provenance: %zu",
            total_, passed_, blocked_subject_, blocked_provenance_);
        if (total_ > 0) {
            const double prov_block_rate =
                100.0 * static_cast<double>(blocked_provenance_) / static_cast<double>(total_);
            RCLCPP_INFO(get_logger(),
                "Provenance block rate (Attack Class 2): %.1f%%", prov_block_rate);
        }
    }

private:
    PolicyRule policy_{};

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;

    std::size_t total_ = 0;
    std::size_t passed_ = 0;
    std::size_t blocked_subject_ = 0;
    std::size_t blocked_provenance_ = 0;

    void on_command(const std_msgs::msg::String::SharedPtr msg) {
        ++total_;

        std::uint16_t subject_id = 0;
        std::uint32_t provenance_mask = 0;

        if (!parse_message(msg->data, subject_id, provenance_mask)) {
            RCLCPP_WARN(get_logger(), "[BLOCK_PARSE] Malformed payload: '%s'",
                        msg->data.c_str());
            return;
        }

        AuthorizationRequest request{};
        request.subject_id  = subject_id;
        request.object_id   = kObject;
        set_bit(request.required_attributes, kAttr);
        request.source_provenance = provenance_mask;

        const auto t0 = std::chrono::high_resolution_clock::now();
        const bool authorized = hngac_authorize(&policy_, 1, request);
        const auto t1 = std::chrono::high_resolution_clock::now();
        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

        if (authorized) {
            ++passed_;
            geometry_msgs::msg::Twist vel{};
            vel.linear.x = 1.0;
            publisher_->publish(vel);
            RCLCPP_INFO(get_logger(),
                "[PASS_5D] subject=%u prov=%u | %ld ns | forwarded",
                subject_id, provenance_mask, ns);
        } else {
            // Distinguish subject-fail from provenance-fail for paper statistics.
            // If the subject is the authorized subject (1) but provenance is wrong, that
            // is Attack Class 2: compromised authenticated node.
            if (subject_id == kSubject) {
                ++blocked_provenance_;
                RCLCPP_WARN(get_logger(),
                    "[BLOCK_PROV] subject=%u prov=%u | %ld ns | Attack Class 2 blocked",
                    subject_id, provenance_mask, ns);
            } else {
                ++blocked_subject_;
                RCLCPP_WARN(get_logger(),
                    "[BLOCK_SUBJ] subject=%u prov=%u | %ld ns | unauthorized subject",
                    subject_id, provenance_mask, ns);
            }
        }
    }
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Gatekeeper5DNode>());
    rclcpp::shutdown();
    return 0;
}

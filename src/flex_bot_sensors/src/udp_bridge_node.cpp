#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>
#include <sensor_msgs/msg/imu.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

using namespace std::chrono_literals;

// -------------------- Helpers --------------------
static inline double radps_to_rpm(double rad_s) {
  return rad_s * 60.0 / (2.0 * M_PI);
}
static inline double rpm_to_radps(double rpm) {
  return rpm * (2.0 * M_PI) / 60.0;
}

static uint32_t crc32(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int b = 0; b < 8; b++) {
      uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

static inline void rpy_to_quat(double roll, double pitch, double yaw,
                               double &qx, double &qy, double &qz, double &qw) {
  const double cr = std::cos(roll * 0.5);
  const double sr = std::sin(roll * 0.5);
  const double cp = std::cos(pitch * 0.5);
  const double sp = std::sin(pitch * 0.5);
  const double cy = std::cos(yaw * 0.5);
  const double sy = std::sin(yaw * 0.5);

  qw = cr*cp*cy + sr*sp*sy;
  qx = sr*cp*cy - cr*sp*sy;
  qy = cr*sp*cy + sr*cp*sy;
  qz = cr*cp*sy - sr*sp*cy;
}

// -------------------- UDP packets --------------------
#pragma pack(push, 1)
struct CmdPacket {
  float left_rpm;
  float right_rpm;
};

struct RpmFeedbackPacket {
  float left_rpm;
  float right_rpm;
  double timestamp;
};

struct UdpImuPacket {
  uint32_t magic;
  uint16_t version;
  uint16_t payload_len;
  uint32_t seq;
  uint64_t t_monotonic_ns;
  float roll;
  float pitch;
  float yaw;
  uint32_t crc;
};

struct UdpBatteryPacket {
  uint32_t magic; // "UBAT"
  uint16_t version;
  uint16_t payload_len;
  uint32_t seq;
  uint64_t t_monotonic_ns;
  float voltage;
  float current;
  float soc;
  float temperature;
  uint32_t crc;
};
#pragma pack(pop)

// ================= NODE =================
class FlexBotUdpBridge : public rclcpp::Node {
public:
  FlexBotUdpBridge() : Node("flex_bot_udp_bridge") {
    imx7_ip_  = declare_parameter<std::string>("imx7_ip", "192.168.0.2");
    cmd_port_ = declare_parameter<int>("cmd_port", 5001);
    fb_port_  = declare_parameter<int>("fb_port", 5002);

    imu_port_ = declare_parameter<int>("imu_port", 5005);
    battery_port_ = declare_parameter<int>("battery_port", 5007);

    imu_frame_id_ = declare_parameter<std::string>("imu_frame_id", "xsens_imu");
    imu_rpy_in_deg_ = declare_parameter<bool>("imu_rpy_in_degrees", false);

    cmd_rate_hz_ = declare_parameter<double>("cmd_rate_hz", 50.0);
    cmds_are_radps_ = declare_parameter<bool>("cmds_are_radps", true);
    publish_feedback_radps_ = declare_parameter<bool>("publish_feedback_radps", false);

    auto qos = rclcpp::QoS(10).best_effort();

    // COMMAND SUBS
    sub_left_cmd_ = create_subscription<std_msgs::msg::Float64>(
      "/left_wheel/cmd_vel", qos,
      [this](const std_msgs::msg::Float64 &m){
        std::lock_guard<std::mutex> lk(cmd_mtx_);
        left_cmd_ = m.data;
      });

    sub_right_cmd_ = create_subscription<std_msgs::msg::Float64>(
      "/right_wheel/cmd_vel", qos,
      [this](const std_msgs::msg::Float64 &m){
        std::lock_guard<std::mutex> lk(cmd_mtx_);
        right_cmd_ = m.data;
      });

    // WHEEL PUBS
    pub_left_rpm_  = create_publisher<std_msgs::msg::Float64>("/left_wheel/vel_rpm", qos);
    pub_right_rpm_ = create_publisher<std_msgs::msg::Float64>("/right_wheel/vel_rpm", qos);

    if (publish_feedback_radps_) {
      pub_left_radps_  = create_publisher<std_msgs::msg::Float64>("/left_wheel/vel_radps", qos);
      pub_right_radps_ = create_publisher<std_msgs::msg::Float64>("/right_wheel/vel_radps", qos);
    }

    // IMU PUB
    pub_imu_ = create_publisher<sensor_msgs::msg::Imu>("/xsens_imu", rclcpp::QoS(50).best_effort());

    // BATTERY PUBS
    pub_battery_voltage_ = create_publisher<std_msgs::msg::Float64>("/battery/voltage", 10);
    pub_battery_current_ = create_publisher<std_msgs::msg::Float64>("/battery/current", 10);
    pub_battery_soc_     = create_publisher<std_msgs::msg::Float64>("/battery/soc", 10);
    pub_battery_temp_    = create_publisher<std_msgs::msg::Float64>("/battery/temperature", 10);

    setup_udp();

    running_.store(true);
    rx_thread_ = std::thread([this]{ wheel_rx_loop(); });
    imu_thread_ = std::thread([this]{ imu_rx_loop(); });
    battery_thread_ = std::thread([this]{ battery_rx_loop(); });

    auto period = std::chrono::duration<double>(1.0 / cmd_rate_hz_);
    cmd_timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::nanoseconds>(period),
      [this]{ send_cmd_once(); });

    RCLCPP_INFO(get_logger(), "UDP bridge ready");
  }

  ~FlexBotUdpBridge() {
    running_.store(false);
    if (rx_thread_.joinable()) rx_thread_.join();
    if (imu_thread_.joinable()) imu_thread_.join();
    if (battery_thread_.joinable()) battery_thread_.join();

    if (sock_tx_ >= 0) close(sock_tx_);
    if (sock_rx_wheel_ >= 0) close(sock_rx_wheel_);
    if (sock_rx_imu_ >= 0) close(sock_rx_imu_);
    if (sock_rx_batt_ >= 0) close(sock_rx_batt_);
  }

private:
  void setup_udp() {
    sock_tx_ = socket(AF_INET, SOCK_DGRAM, 0);

    imx7_addr_.sin_family = AF_INET;
    imx7_addr_.sin_port = htons(cmd_port_);
    inet_pton(AF_INET, imx7_ip_.c_str(), &imx7_addr_.sin_addr);

    int reuse = 1;

    sock_rx_wheel_ = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in w{};
    w.sin_family = AF_INET;
    w.sin_port = htons(fb_port_);
    w.sin_addr.s_addr = INADDR_ANY;
    setsockopt(sock_rx_wheel_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bind(sock_rx_wheel_, (sockaddr*)&w, sizeof(w));

    sock_rx_imu_ = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in i{};
    i.sin_family = AF_INET;
    i.sin_port = htons(imu_port_);
    i.sin_addr.s_addr = INADDR_ANY;
    setsockopt(sock_rx_imu_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bind(sock_rx_imu_, (sockaddr*)&i, sizeof(i));

    sock_rx_batt_ = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in b{};
    b.sin_family = AF_INET;
    b.sin_port = htons(battery_port_);
    b.sin_addr.s_addr = INADDR_ANY;
    setsockopt(sock_rx_batt_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bind(sock_rx_batt_, (sockaddr*)&b, sizeof(b));
  }

  void send_cmd_once() {
    double l, r;
    {
      std::lock_guard<std::mutex> lk(cmd_mtx_);
      l = left_cmd_;
      r = right_cmd_;
    }

    CmdPacket pkt{};
    pkt.left_rpm  = cmds_are_radps_ ? radps_to_rpm(l) : l;
    pkt.right_rpm = cmds_are_radps_ ? radps_to_rpm(r) : r;

    sendto(sock_tx_, &pkt, sizeof(pkt), 0,
           (sockaddr*)&imx7_addr_, sizeof(imx7_addr_));
  }

  void wheel_rx_loop() {
    RpmFeedbackPacket pkt{};
    while (running_) {
      if (recv(sock_rx_wheel_, &pkt, sizeof(pkt), 0) != sizeof(pkt)) continue;

      std_msgs::msg::Float64 m;
      m.data = pkt.left_rpm;  pub_left_rpm_->publish(m);
      m.data = pkt.right_rpm; pub_right_rpm_->publish(m);

      if (publish_feedback_radps_) {
        m.data = rpm_to_radps(pkt.left_rpm);  pub_left_radps_->publish(m);
        m.data = rpm_to_radps(pkt.right_rpm); pub_right_radps_->publish(m);
      }
    }
  }

  void imu_rx_loop() {
    UdpImuPacket pkt{};
    while (running_) {
      if (recv(sock_rx_imu_, &pkt, sizeof(pkt), 0) != sizeof(pkt)) continue;

      uint32_t crc_rx = pkt.crc;
      pkt.crc = 0;
      if (crc32((uint8_t*)&pkt, sizeof(pkt)-4) != crc_rx) continue;

      double r = pkt.roll, p = pkt.pitch, y = pkt.yaw;
      if (imu_rpy_in_deg_) { r*=M_PI/180; p*=M_PI/180; y*=M_PI/180; }

      double qx,qy,qz,qw;
      rpy_to_quat(r,p,y,qx,qy,qz,qw);

      sensor_msgs::msg::Imu imu;
      imu.header.stamp = now();
      imu.header.frame_id = imu_frame_id_;
      imu.orientation.x = qx;
      imu.orientation.y = qy;
      imu.orientation.z = qz;
      imu.orientation.w = qw;

      pub_imu_->publish(imu);
    }
  }

  void battery_rx_loop() {
    UdpBatteryPacket pkt{};
    while (running_) {
      if (recv(sock_rx_batt_, &pkt, sizeof(pkt), 0) != sizeof(pkt)) continue;

      uint32_t crc_rx = pkt.crc;
      pkt.crc = 0;
      if (crc32((uint8_t*)&pkt, sizeof(pkt)-4) != crc_rx) continue;

      std_msgs::msg::Float64 m;

      m.data = pkt.voltage; pub_battery_voltage_->publish(m);
      m.data = pkt.current; pub_battery_current_->publish(m);
      m.data = pkt.soc;     pub_battery_soc_->publish(m);
      m.data = pkt.temperature; pub_battery_temp_->publish(m);
    }
  }

private:
  std::string imx7_ip_;
  int cmd_port_, fb_port_, imu_port_, battery_port_;

  std::string imu_frame_id_;
  bool imu_rpy_in_deg_;
  double cmd_rate_hz_;
  bool cmds_are_radps_;
  bool publish_feedback_radps_;

  std::mutex cmd_mtx_;
  double left_cmd_{0}, right_cmd_{0};

  int sock_tx_{-1}, sock_rx_wheel_{-1}, sock_rx_imu_{-1}, sock_rx_batt_{-1};
  sockaddr_in imx7_addr_{};

  std::atomic<bool> running_{false};
  std::thread rx_thread_, imu_thread_, battery_thread_;

  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr sub_left_cmd_, sub_right_cmd_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_left_rpm_, pub_right_rpm_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_left_radps_, pub_right_radps_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr pub_imu_;

  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_battery_voltage_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_battery_current_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_battery_soc_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pub_battery_temp_;

  rclcpp::TimerBase::SharedPtr cmd_timer_;
};

// ================= MAIN =================
int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FlexBotUdpBridge>());
  rclcpp::shutdown();
  return 0;
}

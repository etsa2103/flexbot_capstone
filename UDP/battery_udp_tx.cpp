
#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

// ================= CONFIG =================
static constexpr const char* CAN_INTERFACE = "can1";
static constexpr uint32_t CAN_ID_PRIMARY   = 0x1B1;
static constexpr uint32_t CAN_ID_TEMP      = 0x4B1;

static constexpr const char* DEST_IP   = "192.168.10.20";
static constexpr int DEST_PORT         = 5007;

static constexpr int SERIES_CELLS      = 15;
static constexpr double NOMINAL_CAPACITY = 30.0;

// ================= CRC32 =================
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

static uint64_t monotonic_ns() {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return uint64_t(ts.tv_sec) * 1000000000ULL + uint64_t(ts.tv_nsec);
}

// ================= UDP PACKET =================
#pragma pack(push,1)
struct UdpBatteryPacket {
    uint32_t magic;          // "UBAT"
    uint16_t version;        // 1
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

// ================= CAN SOCKET =================
class CANSocket {
public:
    CANSocket() : fd_(-1) {}

    ~CANSocket() {
        if (fd_ >= 0) close(fd_);
    }

    bool open(const char* iface) {
        fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (fd_ < 0) {
            perror("socket");
            return false;
        }

        struct ifreq ifr{};
        strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

        if (ioctl(fd_, SIOCGIFINDEX, &ifr) < 0) {
            perror("ioctl");
            return false;
        }

        sockaddr_can addr{};
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            return false;
        }

        timeval tv{};
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        return true;
    }

    bool recv(can_frame& frame) {
        return read(fd_, &frame, sizeof(frame)) == sizeof(frame);
    }

private:
    int fd_;
};

// ================= SOC ESTIMATION =================
static float voltage_to_soc(float pack_voltage) {
    float cell_v = pack_voltage / SERIES_CELLS;

    if (cell_v >= 3.40f) return 100.0f;
    if (cell_v >= 3.35f) return 90.0f;
    if (cell_v >= 3.32f) return 70.0f;
    if (cell_v >= 3.30f) return 40.0f;
    if (cell_v >= 3.27f) return 30.0f;
    if (cell_v >= 3.25f) return 20.0f;
    if (cell_v >= 3.20f) return 10.0f;
    if (cell_v >= 3.10f) return 5.0f;
    return 0.0f;
}

// ================= MAIN =================
int main() {
    CANSocket can;
    if (!can.open(CAN_INTERFACE)) {
        fprintf(stderr, "Failed to open %s\n", CAN_INTERFACE);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("udp socket");
        return 1;
    }

    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(DEST_PORT);
    inet_pton(AF_INET, DEST_IP, &dst.sin_addr);

    float voltage = 0.0f;
    float current = 0.0f;
    float soc = 0.0f;
    float temp = 0.0f;

    uint32_t seq = 0;

    printf("Battery UDP TX -> %s:%d\n", DEST_IP, DEST_PORT);

    while (true) {
        can_frame frame{};

        if (!can.recv(frame))
            continue;

        if (frame.can_id == CAN_ID_PRIMARY && frame.can_dlc >= 8) {
            uint16_t v_raw = frame.data[0] | (frame.data[1] << 8);
            int16_t  c_raw = frame.data[4] | (frame.data[5] << 8);

            voltage = v_raw / 256.0f;
            current = c_raw / 10.0f;
            soc = voltage_to_soc(voltage);
        }

        if (frame.can_id == CAN_ID_TEMP && frame.can_dlc >= 2) {
            uint16_t t_raw = frame.data[0] | (frame.data[1] << 8);
            temp = t_raw / 100.0f;
        }

        UdpBatteryPacket pkt{};
        pkt.magic = 0x54414255u; // UBAT
        pkt.version = 1;
        pkt.payload_len = sizeof(UdpBatteryPacket);
        pkt.seq = seq++;
        pkt.t_monotonic_ns = monotonic_ns();
        pkt.voltage = voltage;
        pkt.current = current;
        pkt.soc = soc;
        pkt.temperature = temp;
        pkt.crc = 0;

        pkt.crc = crc32(reinterpret_cast<uint8_t*>(&pkt),
                        sizeof(UdpBatteryPacket) - sizeof(uint32_t));

        sendto(sock, &pkt, sizeof(pkt), 0,
               (sockaddr*)&dst, sizeof(dst));
    }

    close(sock);
    return 0;
}
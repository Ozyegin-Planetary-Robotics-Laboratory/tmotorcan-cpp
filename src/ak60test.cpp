#include "ak60.hpp"

// int main(int argc, char **argv) {
//   const uint8_t motor_id = 0x0B;
//   int _can_fd;

//   /* create socket file descriptor */
//   _can_fd = -1;
//   while (_can_fd < 0) {
//     if ((_can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
//       perror("Error while opening socket");
//       std::chrono::seconds dura(1);
//     }
//   }

//   /* input the correct network interface name */
//   struct ifreq ifr;
//   strcpy(ifr.ifr_name, "can0");
//   ioctl(_can_fd, SIOCGIFINDEX, &ifr);

//   /* create the socket address and bind the interface to it */
//   struct sockaddr_can addr;
//   addr.can_family = AF_CAN;
//   addr.can_ifindex = ifr.ifr_ifindex;
//   int bind = -1;
//   while (bind < 0) {
//     if ((bind = ::bind(_can_fd, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
//       perror("Error in socket bind");
//       std::chrono::seconds dura(1);
//     }
//   }

//   /* Filter for the CAN messages. */
//   struct can_filter rfilter;
//   rfilter.can_id = 0x00002900 | motor_id;
//   rfilter.can_mask = CAN_SFF_MASK;
//   setsockopt(_can_fd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(struct can_filter));

//   bool *shutdown = new bool(false);
//   std::thread _can_reader = std::thread([_can_fd, shutdown] {
//     while (!(*shutdown)) {
//       struct can_frame frame;
//       int nbytes = read(_can_fd, &frame, sizeof(struct can_frame));
//       if (nbytes < 0) {
//         perror("Error in reading");
//       }
//       float position = ((int16_t) (frame.data[0] << 8 | frame.data[1])) / 10.0;
//       printf("Position: %f\n", position);
//     }
//   });
//   _can_reader.detach();

//   for (int i = 0; i < 10; i++) {
//     float position = i*45.0f;
//     struct can_frame frame;
//     frame.can_id = 0x00000400 | motor_id | CAN_EFF_FLAG;
//     int32_t pos_cmd_int = (int32_t) (position * 10000.0f);
//     frame.data[0] = (pos_cmd_int >> 24) & 0xFF;
//     frame.data[1] = (pos_cmd_int >> 16) & 0xFF;
//     frame.data[2] = (pos_cmd_int >> 8) & 0xFF;
//     frame.data[3] = pos_cmd_int & 0xFF;
//     frame.can_dlc = 4;
//     int nbytes = write(_can_fd, &frame, sizeof(struct can_frame));
//     if (nbytes < 0) {
//       perror("Error in writing");
//     }
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//   }

//   *shutdown = true;
//   return 0;
// }

int main(int argc, char **argv) {
  TMotor::AK60Manager ak60(0x0B, "can0");
  ak60.setPosition(-45.0f);
  return 0;
}
#ifndef POLLDEVICE_HH
#define POLLDEVICE_HH

/*
 * =c
 * PollDevice(devname)
 * =d
 * 
 * Poll packets received by the Linux network interface named devname.
 * Packets can be pulled from output 0. The packets include the link-level
 * header.
 *
 * Linux won't see any packets from the device. If you want Linux to process
 * packets, you should hand them to ToLinux.
 *
 * This element is only available inside the kernel module.
 *
 * =a FromDevice
 * =a ToDevice
 * =a ToLinux
 */

#include "element.hh"
#include "string.hh"
#include "glue.hh"

class PollDevice : public Element {
  
 public:
  
  PollDevice();
  PollDevice(const String &);
  ~PollDevice();
  
  static void static_initialize();
  static void static_cleanup();
  
  const char *class_name() const		{ return "PollDevice"; }
  Processing default_processing() const	{ return PULL; }
  
  PollDevice *clone() const;
  int configure(const String &, ErrorHandler *);
  int initialize(ErrorHandler *);
  void uninitialize();
  
  /* process a packet. return 0 if not wanted after all. */
  int got_skb(struct sk_buff *);

  struct wait_queue** get_wait_queue(); 
  void do_waiting();
  void finish_waiting();

  Packet *pull(int port);
  
 private:

  int total_intr_wait;
  int idle;
  String _devname;
  struct device *_dev;
};

#endif 


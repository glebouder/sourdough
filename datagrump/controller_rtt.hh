#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>
#include <list>

using namespace std;

/* Congestion controller interface */

class Controller
{
private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */

  /* size of the window */
  unsigned int window_size_priv;

  /* How do we increase the window size for each ack received */
  unsigned int add_incr;

  /* How do we decrease the window size when an ack is missing */
  unsigned int add_decr;

  /* list of data sent with no ack received */
  list< pair< const uint64_t , const uint64_t > > expected_ack_list;

  /* If the RTT goes under this value, increase the window size */
  unsigned int threshold_down_ms;

  /* If the RTT goes over this value, decrease the window size  */
  unsigned int threshold_up_ms;

public:
  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */

  /* Default constructor */
  Controller( const bool debug );

  /* Get current window size, in datagrams */
  unsigned int window_size( void );

  /* A datagram was sent */
  void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp );

  /* An ack was received */
  void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received );

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms( void );

  /* check expected_ack_list : if one came too late it means
     we have to decrease the window size */
  void check_list( void );


};

#endif

#include <iostream>

#include "controller.hh"
#include "timestamp.hh"
#include <list>

using namespace std;


/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ),
window_size_priv( 100 ),
  add_incr( 1 ),
  add_decr( 5 ),
  expected_ack_list( list< pair< const uint64_t , const uint64_t > >() ),
  threshold_down_ms( 20 ),
  threshold_up_ms( 100 )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << window_size_priv << endl;
  }

  return window_size_priv;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }

  /* Remove the previous wait of the list */
  expected_ack_list.remove_if
    (
     [ sequence_number ] (pair< const uint64_t , const uint64_t > sent)
     { return (sent.first == sequence_number ); } 
    );

  /* Add the new wait to the list */
  expected_ack_list.push_back( make_pair( sequence_number , send_timestamp ) );

  /* Check the list to see if there is no problem */
  check_list();
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
  /* Remove this ack from the waiting list */
  expected_ack_list.remove_if
    (
     [ sequence_number_acked ] (pair< const uint64_t , const uint64_t > sent)
     { return (sent.first == sequence_number_acked ); } 
    );

  /* Increase the window */
  window_size_priv += add_incr ;

}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}


/* check expected_ack_list : if one came too late it means we
   have to decrease the window size */
void Controller::check_list( void )
{
  bool must_incr = false;
  bool must_decr = false;
  list< pair< const uint64_t , const uint64_t > >::iterator it;
  for (it=expected_ack_list.begin(); it != expected_ack_list.end(); ++it)
  {
    must_incr = must_incr || ( timestamp_ms() - (*it).second <= threshold_down_ms );
    must_decr = must_decr || ( timestamp_ms() - (*it).second >= threshold_up_ms );
  }
  if ( must_decr ) { window_size_priv = window_size_priv - add_decr; }
  else if ( must_incr ) { window_size_priv = window_size_priv + add_incr; }
}





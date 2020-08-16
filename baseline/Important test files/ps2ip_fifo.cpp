#include "my_ip_hls.hpp"


void ps2ip_fifo(stream<axiWord> &ps2ip,stream<axiWord> &ps2ipIntFifo) {
#pragma HLS PIPELINE II=1 enable_flush

	static enum inStates 	{	IN_STATE_IDLE = 0
							} curState;


	switch(curState) {

		  case (IN_STATE_IDLE): {
			  if (!ps2ip.empty()) {
				  axiWord newInWord = {0,0,0};
				  ps2ip.read(newInWord);
				  ps2ipIntFifo.write(newInWord);
			  }

			  curState = IN_STATE_IDLE;
			  break;
	      }
	};

	return;

}






#include "my_ip_hls.hpp"


void ip2ps_fifo(stream<axiWord> &ip2psIntFifo,stream<axiWord> &ip2ps) {
#pragma HLS PIPELINE II=1 enable_flush

	static enum inStates 	{	IN_STATE_IDLE = 0
							} curState;


	switch(curState) {

		  case (IN_STATE_IDLE): {
			  if (!ip2psIntFifo.empty()) {
				  axiWord newInWord = {0,0,0};
				  ip2psIntFifo.read(newInWord);
				  ip2ps.write(newInWord);
			  }

			  curState = IN_STATE_IDLE;
			  break;
	      }
	};

	return;

}






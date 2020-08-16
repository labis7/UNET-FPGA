#include "my_ip_hls.hpp"


void core(stream<axiWord> &ps2ipIntFifo,stream<axiWord> &ip2psIntFifo, uint32 rule1,uint32 rule2,uint32 &count_out) {
#pragma HLS PIPELINE II=1 enable_flush
#pragma HLS INTERFACE ap_ctrl_none port=return

	static enum inStates 	{
								IN_STATE_IDLE = 0
							} curState;

	static axiWord newInWord = {0,0,0};

	switch(curState) {

		  case (IN_STATE_IDLE): {
			  if (!ps2ipIntFifo.empty()) {
				  ps2ipIntFifo.read(newInWord);

				  //TODO: data processing

				  if(rule1 == newInWord.data)
				  {
					  newInWord.data = 200;
				  	  count_out =5;
				  }
				  if(rule2 == newInWord.data)
				  {
					  newInWord.data = 500;
				  	  count_out =5;
				  }

				  ip2psIntFifo.write(newInWord);
			  }

			  curState = IN_STATE_IDLE;

			  break;
	      }
	};

	return;

}






#include "my_ip_hls.hpp"

static uint32 rule1;
static uint32 rule2;
void rules(uint32 rule_1, uint32 rule_2)
{

	rule1 = rule_1;
	rule2 = rule_2;

}

void get_rules(uint32 &tmp1,uint32 &tmp2)
{
	tmp1 = rule1;
	tmp2 = rule2;
}
/*
void get_rules(uint32 &rule_out)
{
	rule_out = rule;
}
*/

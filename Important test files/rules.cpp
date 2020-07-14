#include "my_ip_hls.hpp"

static uint32 rule;
static uint32 rule2;
void rules(uint32 rule1, uint32 rule_2)
{

	rule = rule1;
	rule2 = rule_2;

}

void get_rules(uint32 &tmp,uint32 &tmp1)
{
	tmp = rule;
	tmp1 = rule2;
}
/*
void get_rules(uint32 &rule_out)
{
	rule_out = rule;
}
*/

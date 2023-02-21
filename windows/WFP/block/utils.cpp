#include "utils.h"

BOOLEAN is_same_ipv4(const IN_ADDR& first, const IN_ADDR& second)
{
	return first.S_un.S_addr == second.S_un.S_addr;
}

BOOLEAN is_same_ipv6(const IN6_ADDR& first, const IN6_ADDR& second)
{
	BOOLEAN result = TRUE;
	for (int i = 0; i < 8; i++) {
		if (first.u.Word[i] != second.u.Word[i]) {
			result = FALSE;
			break;
		}
	}
	return result;
}

VOID host2net_ipv4(IN_ADDR& ip) {
	net2host_ipv4(ip);
}

VOID host2net_ipv6(IN6_ADDR& ip) {
	net2host_ipv6(ip);
}

VOID net2host_ipv4(IN_ADDR& ip) {
	UCHAR tmp;

	tmp = ip.S_un.S_un_b.s_b1;
	ip.S_un.S_un_b.s_b1 = ip.S_un.S_un_b.s_b4;
	ip.S_un.S_un_b.s_b4 = tmp;

	tmp = ip.S_un.S_un_b.s_b2;
	ip.S_un.S_un_b.s_b2 = ip.S_un.S_un_b.s_b3;
	ip.S_un.S_un_b.s_b3 = tmp;

	return;
}

VOID net2host_ipv6(IN6_ADDR& ip) {
	UCHAR tmp;
	for (int i = 0, j = 7; i < j; i++, j--) {
		tmp = ip.u.Byte[i];
		ip.u.Byte[j] = ip.u.Byte[i];
		ip.u.Byte[i] = tmp;
	}

	return;
}
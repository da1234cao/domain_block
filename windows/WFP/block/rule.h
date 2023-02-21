#pragma once
#include "common.h"

#define MAX_IPV4_LIEN 20
#define MAX_IPV6_LIEN 50//ipv6����󳤶�Ϊ46

#define WDF_BLOCK_TAG 'colb'

extern LIST_ENTRY rule_list;
extern WDFWAITLOCK rule_list_lock;

// �����û��ռ�ʹ����ͬ��rule�ṹ�ϣ������ں�list�ṹ
typedef struct _rule_node_t {
	LIST_ENTRY list_entry;
	rule_t rule;
}rule_node_t;

// �ں˲��񵽵���Ϣ��������rule���Ƚ�
typedef struct _connect_info_t {
	int ip_version;
	union {
		struct {
			IN_ADDR local_address;
			IN_ADDR remote_address;
			USHORT local_port;
			USHORT remote_port;
		}v4;
		struct {
			IN6_ADDR local_address;
			IN6_ADDR remote_address;
			USHORT local_port;
			USHORT remote_port;
			// SCOPE_ID remote_scope_id; // Ŀǰ�����ipv6�е�scop_id
		}v6;
	};
	UINT64 process_id;
}connect_info_t;

NTSTATUS init_rule();

NTSTATUS add_rule(rule_t &rule);

NTSTATUS del_rule(rule_t& rule);

NTSTATUS clear_rules();

BOOLEAN is_hint_rule(connect_info_t &connect_info);

rule_t& reverse_rule(rule_t& rule);
#include "wfp_callbacks.h"
#include "wfp.h"
#include "utils.h"

void callout_classify(
	const FWPS_INCOMING_VALUES* inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	void* layerData,
	const void* classifyContext,
	const FWPS_FILTER* filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT* classifyOut)
{
	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(flowContext);

	if (!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE)){
		return;
	}

	KdPrint(("|LIBBLOCK|callout_classify|layerId: %d", inFixedValues->layerId));
	connect_info_t conn;
	conn.process_id = inMetaValues->processId;
	if (inFixedValues->layerId == FWPS_LAYER_ALE_FLOW_ESTABLISHED_V4)
	{
		conn.ip_version = 4;
		// FWP_UINT32: 主机序
		conn.v4.local_address =
			*reinterpret_cast<IN_ADDR*>(&inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_ADDRESS].value.uint32);
		// FWP_UINT32
		conn.v4.remote_address =
			*reinterpret_cast<IN_ADDR*>(&inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_ADDRESS].value.uint32);
		// FWP_UINT16: 主机序
		conn.v4.local_port =
			inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_PORT].value.uint16;
		// FWP_UINT16
		conn.v4.remote_port
			= inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_PORT].value.uint16;

		KdPrint(("|LIBBLOCK|callout_classify|IPv4, %d.%d.%d.%d:%hu --> %d.%d.%d.%d:%hu, PID: %lld",
			FORMAT_ADDR4(conn.v4.local_address), conn.v4.local_port,
			FORMAT_ADDR4(conn.v4.remote_address), conn.v4.remote_port,
			conn.process_id));
	}
	else if (inFixedValues->layerId == FWPS_LAYER_ALE_FLOW_ESTABLISHED_V6)
	{
		conn.ip_version = 6;
		// FWP_BYTE_ARRAY16_TYPE
		conn.v6.local_address =
			*reinterpret_cast<IN6_ADDR*>(inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V6_IP_LOCAL_ADDRESS].value.byteArray16);
		// FWP_BYTE_ARRAY16_TYPE
		conn.v6.remote_address =
			*reinterpret_cast<IN6_ADDR*>(inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V6_IP_REMOTE_ADDRESS].value.byteArray16);
		// FWP_UINT16
		conn.v6.local_port =
			inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V6_IP_LOCAL_PORT].value.uint16;
		// FWP_UINT16
		conn.v6.remote_port =
			inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V6_IP_REMOTE_PORT].value.uint16;

		// conn.v6.remote_scope_id = inMetaValues->remoteScopeId;

		KdPrint(("|LIBBLOCK|callout_classify|IPv6, %x:%x:%x:%x:%x:%x:%x::%x:%hu --> %x:%x:%x:%x:%x:%x:%x::%x:%hu, PID: %lld",
			FORMAT_ADDR6(conn.v6.local_address), conn.v6.local_port,
			FORMAT_ADDR6(conn.v6.remote_address), conn.v6.remote_port,
			conn.process_id));
	}
	else
	{
		KdPrint(("|LIBBLCO|callout_classify|invalid filter context"));
		classifyOut->actionType = FWP_ACTION_PERMIT;
		return;
	}

	classifyOut->actionType = FWP_ACTION_PERMIT;
	if (is_hint_rule(conn) == TRUE) {
		classifyOut->actionType = FWP_ACTION_BLOCK;
	}

	if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT) {
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
	}

	return;
}



NTSTATUS NTAPI callout_notify(
	FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	const GUID* filterKey,
	FWPS_FILTER* filter)
{
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	NTSTATUS status = STATUS_SUCCESS;
	switch (notifyType) {
	case FWPS_CALLOUT_NOTIFY_ADD_FILTER:
		KdPrint(("|LIBBLOCK|callout_notify|A new filter has registered, filterId: %lld", filter->filterId));
		break;
	case FWPS_CALLOUT_NOTIFY_DELETE_FILTER:
		// 删除一个filter时，将对应链表中相应的规则清楚
		KdPrint(("|LIBBLOCK|callout_notify|A filter has just been deleted, filterId: %lld", filter->filterId));
		// tood!
		break;
	}
	return status;
}


void NTAPI callout_flow_delete(UINT16 layerId, UINT32 calloutId, UINT64 flowContext)
{
	UNREFERENCED_PARAMETER(layerId);
	UNREFERENCED_PARAMETER(calloutId);
	UNREFERENCED_PARAMETER(flowContext);
}
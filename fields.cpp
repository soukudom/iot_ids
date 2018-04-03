/************* THIS IS AUTOMATICALLY GENERATED FILE, DO NOT EDIT *************/
// Tables are indexed by field ID
#include "fields.h"

char *ur_field_names_static[] = {
   "ACKCount",
   "ACKWaiting",
   "aclPackets",
   "address",
   "average",
   "averageRequestRTT",
   "averageResponseRTT",
   "badChecksum",
   "badroutes",
   "broadcastReadCount",
   "broadcastWriteCount",
   "callbacks",
   "CANCount",
   "cum_average",
   "dropped",
   "err_value",
   "GW_ID",
   "ID",
   "lastRequestRTT",
   "lastResponseRTT",
   "median",
   "NAKCount",
   "netBusy",
   "noACK",
   "nodeID",
   "nonDelivery",
   "notIdle",
   "OOFCount",
   "profile_value",
   "quality",
   "readAborts",
   "readCount",
   "receivedCount",
   "receiveDuplications",
   "receiveUnsolicited",
   "retries",
   "routedBusy",
   "rxAcls",
   "rxBytes",
   "rxErrors",
   "rxEvents",
   "rxScos",
   "scoMtu",
   "scoPackets",
   "sentCount",
   "sentFailed",
   "SOAFCount",
   "TIME",
   "txAcls",
   "txBytes",
   "txCmds",
   "txErrors",
   "txScos",
   "VALUE",
   "variance",
   "writeCount",
   "alert_desc",
   "profile_key",
   "ur_key",
};
short ur_field_sizes_static[] = {
   8, /* ACKCount */
   8, /* ACKWaiting */
   8, /* aclPackets */
   8, /* address */
   8, /* average */
   8, /* averageRequestRTT */
   8, /* averageResponseRTT */
   8, /* badChecksum */
   8, /* badroutes */
   8, /* broadcastReadCount */
   8, /* broadcastWriteCount */
   8, /* callbacks */
   8, /* CANCount */
   8, /* cum_average */
   8, /* dropped */
   8, /* err_value */
   8, /* GW_ID */
   8, /* ID */
   8, /* lastRequestRTT */
   8, /* lastResponseRTT */
   8, /* median */
   8, /* NAKCount */
   8, /* netBusy */
   8, /* noACK */
   8, /* nodeID */
   8, /* nonDelivery */
   8, /* notIdle */
   8, /* OOFCount */
   8, /* profile_value */
   8, /* quality */
   8, /* readAborts */
   8, /* readCount */
   8, /* receivedCount */
   8, /* receiveDuplications */
   8, /* receiveUnsolicited */
   8, /* retries */
   8, /* routedBusy */
   8, /* rxAcls */
   8, /* rxBytes */
   8, /* rxErrors */
   8, /* rxEvents */
   8, /* rxScos */
   8, /* scoMtu */
   8, /* scoPackets */
   8, /* sentCount */
   8, /* sentFailed */
   8, /* SOAFCount */
   8, /* TIME */
   8, /* txAcls */
   8, /* txBytes */
   8, /* txCmds */
   8, /* txErrors */
   8, /* txScos */
   8, /* VALUE */
   8, /* variance */
   8, /* writeCount */
   -1, /* alert_desc */
   -1, /* profile_key */
   -1, /* ur_key */
};
ur_field_type_t ur_field_types_static[] = {
   UR_TYPE_DOUBLE, /* ACKCount */
   UR_TYPE_DOUBLE, /* ACKWaiting */
   UR_TYPE_DOUBLE, /* aclPackets */
   UR_TYPE_DOUBLE, /* address */
   UR_TYPE_DOUBLE, /* average */
   UR_TYPE_DOUBLE, /* averageRequestRTT */
   UR_TYPE_DOUBLE, /* averageResponseRTT */
   UR_TYPE_DOUBLE, /* badChecksum */
   UR_TYPE_DOUBLE, /* badroutes */
   UR_TYPE_DOUBLE, /* broadcastReadCount */
   UR_TYPE_DOUBLE, /* broadcastWriteCount */
   UR_TYPE_DOUBLE, /* callbacks */
   UR_TYPE_DOUBLE, /* CANCount */
   UR_TYPE_DOUBLE, /* cum_average */
   UR_TYPE_DOUBLE, /* dropped */
   UR_TYPE_DOUBLE, /* err_value */
   UR_TYPE_UINT64, /* GW_ID */
   UR_TYPE_UINT64, /* ID */
   UR_TYPE_DOUBLE, /* lastRequestRTT */
   UR_TYPE_DOUBLE, /* lastResponseRTT */
   UR_TYPE_DOUBLE, /* median */
   UR_TYPE_DOUBLE, /* NAKCount */
   UR_TYPE_DOUBLE, /* netBusy */
   UR_TYPE_DOUBLE, /* noACK */
   UR_TYPE_DOUBLE, /* nodeID */
   UR_TYPE_DOUBLE, /* nonDelivery */
   UR_TYPE_DOUBLE, /* notIdle */
   UR_TYPE_DOUBLE, /* OOFCount */
   UR_TYPE_DOUBLE, /* profile_value */
   UR_TYPE_DOUBLE, /* quality */
   UR_TYPE_DOUBLE, /* readAborts */
   UR_TYPE_DOUBLE, /* readCount */
   UR_TYPE_DOUBLE, /* receivedCount */
   UR_TYPE_DOUBLE, /* receiveDuplications */
   UR_TYPE_DOUBLE, /* receiveUnsolicited */
   UR_TYPE_DOUBLE, /* retries */
   UR_TYPE_DOUBLE, /* routedBusy */
   UR_TYPE_DOUBLE, /* rxAcls */
   UR_TYPE_DOUBLE, /* rxBytes */
   UR_TYPE_DOUBLE, /* rxErrors */
   UR_TYPE_DOUBLE, /* rxEvents */
   UR_TYPE_DOUBLE, /* rxScos */
   UR_TYPE_DOUBLE, /* scoMtu */
   UR_TYPE_DOUBLE, /* scoPackets */
   UR_TYPE_DOUBLE, /* sentCount */
   UR_TYPE_DOUBLE, /* sentFailed */
   UR_TYPE_DOUBLE, /* SOAFCount */
   UR_TYPE_DOUBLE, /* TIME */
   UR_TYPE_DOUBLE, /* txAcls */
   UR_TYPE_DOUBLE, /* txBytes */
   UR_TYPE_DOUBLE, /* txCmds */
   UR_TYPE_DOUBLE, /* txErrors */
   UR_TYPE_DOUBLE, /* txScos */
   UR_TYPE_DOUBLE, /* VALUE */
   UR_TYPE_DOUBLE, /* variance */
   UR_TYPE_DOUBLE, /* writeCount */
   UR_TYPE_STRING, /* alert_desc */
   UR_TYPE_STRING, /* profile_key */
   UR_TYPE_STRING, /* ur_key */
};
ur_static_field_specs_t UR_FIELD_SPECS_STATIC = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 59};
ur_field_specs_t ur_field_specs = {ur_field_names_static, ur_field_sizes_static, ur_field_types_static, 59, 59, 59, NULL, UR_UNINITIALIZED};

/** @file moal_sta_cfg80211.c
  *
  * @brief This file contains the functions for STA CFG80211. 
  *
  * Copyright (C) 2011-2012, Marvell International Ltd. 
  * 
  * This software file (the "File") is distributed by Marvell International 
  * Ltd. under the terms of the GNU General Public License Version 2, June 1991 
  * (the "License").  You may use, redistribute and/or modify this File in 
  * accordance with the terms and conditions of the License, a copy of which 
  * is available by writing to the Free Software Foundation, Inc.,
  * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
  * worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
  *
  * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE 
  * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about 
  * this warranty disclaimer.
  *
  */
#include "moal_cfg80211.h"
#include "cfg80211.h"

#include "moal_sta_cfg80211.h"
#ifdef STA_SUPPORT
//static int woal_cfg80211_reg_notifier(struct wiphy *wiphy,
//                                     struct regulatory_request *request);

static int woal_cfg80211_scan(struct wiphy *wiphy, struct net_device *dev,
                              struct cfg80211_scan_request *request);

static int woal_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
                                 struct cfg80211_connect_params *sme);

static int woal_cfg80211_disconnect(struct wiphy *wiphy,
                                    struct net_device *dev, t_u16 reason_code);

static int woal_cfg80211_get_station(struct wiphy *wiphy,
                                     struct net_device *dev,
                                     const u8 * mac, struct station_info *sinfo);

static int woal_cfg80211_dump_station(struct wiphy *wiphy,
                                      struct net_device *dev, int idx,
                                      t_u8 * mac, struct station_info *sinfo);

static int woal_cfg80211_set_power_mgmt(struct wiphy *wiphy,
                                        struct net_device *dev, bool enabled,
                                        int timeout);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35) || defined(COMPAT_WIRELESS)
static int woal_cfg80211_set_cqm_rssi_config(struct wiphy *wiphy,
                                             struct net_device *dev,
                                             s32 rssi_thold, u32 rssi_hyst);
#endif

int woal_cfg80211_set_tx_power(struct wiphy *wiphy,
									  struct net_device *dev,
                                      enum nl80211_tx_power_setting type,
                                      int dbm);

static int woal_cfg80211_join_ibss(struct wiphy *wiphy,
                                   struct net_device *dev,
                                   struct cfg80211_ibss_params *params);

static int woal_cfg80211_leave_ibss(struct wiphy *wiphy,
                                    struct net_device *dev);

#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
static int woal_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
                                             struct net_device *dev,
                                             u64 cookie);

void woal_cfg80211_remain_on_channel_done(void *context);

static int woal_cfg80211_remain_on_channel(struct wiphy *wiphy,
                                           struct net_device *dev,
                                           struct ieee80211_channel *chan,
                                           enum nl80211_channel_type
                                           channel_type, unsigned int duration,
                                           u64 * cookie);

static int woal_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
                                                  struct net_device *dev,
                                                  u64 cookie);
#endif /* KERNEL_VERSION */
#endif /* WIFI_DIRECT_SUPPORT && V14_FEATURE */

/** cfg80211 STA operations */

const struct cfg80211_ops woal_cfg80211_sta_ops = {
	/*.suspend = 0,
	.resume = 0,
	.set_wakeup = 0,
	.add_virtual_intf = 0,
	.del_virtual_intf = 0,*/
    /*.change_virtual_intf = */woal_cfg80211_change_virtual_intf,
    /*.add_key = */woal_cfg80211_add_key,
    /*.get_key = 0,*/
    /*.del_key = */woal_cfg80211_del_key,
    /*.set_default_key = */woal_cfg80211_set_default_key,
    /*.set_default_mgmt_key = 0,*/
	/*.start_ap = */0,
    /*.change_beacon = */0,
    /*.stop_ap = */0,
	/*.add_station = 0,
	.del_station = 0,
	.change_station = 0,*/
	/*.get_station = */woal_cfg80211_get_station,
	/*.dump_station = */woal_cfg80211_dump_station,
	/*.add_mpath = 0,
	.del_mpath = 0,
	.change_mpath = 0,
	.get_mpath = 0,
	.dump_mpath = 0,
	.get_mesh_config = 0,
	.update_mesh_config = 0,
	.join_mesh = 0,
	.leave_mesh = 0,
	.change_bss = 0,
	.set_txq_params = 0,
	.libertas_set_mesh_channel = 0,
	.set_monitor_channel = 0,*/
    /*.scan = */woal_cfg80211_scan,
    /*.auth = 0,
	.assoc = 0,
	.deauth = 0,
	.disassoc = 0,*/
   /* .connect = */woal_cfg80211_connect,
    /*.disconnect = */woal_cfg80211_disconnect,
	/*.join_ibss = */woal_cfg80211_join_ibss,
    /*.leave_ibss = */woal_cfg80211_leave_ibss,
	/*.set_mcast_rate = 0,*/
    /*.set_wiphy_params = */woal_cfg80211_set_wiphy_params,
    /*.set_tx_power = */woal_cfg80211_set_tx_power,
    /*.get_tx_power = 0,
    .set_wds_peer = 0,
    .rfkill_poll = 0,
    .set_bitrate_mask = 0,*/

	/*.mgmt_tx = */woal_cfg80211_mgmt_tx,
	/*.mgmt_tx_cancel_wait = */0,
    /*.remain_on_channel = */0,
    /*.cancel_remain_on_channel = */0,
    /*.set_power_mgmt = */woal_cfg80211_set_power_mgmt,
    /*.set_cqm_rssi_config = */woal_cfg80211_set_cqm_rssi_config,
    
    
    /*.mgmt_frame_register = */woal_cfg80211_mgmt_frame_register,
    /*.set_channel = */woal_cfg80211_set_channel,
};

#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
/** cfg80211 Wifi Direct operations */
const static struct cfg80211_ops woal_cfg80211_wifi_direct_ops = {
	/*.suspend = 0,
	.resume = 0,
	.set_wakeup = 0,
	.add_virtual_intf = 0,
	.del_virtual_intf = 0,*/
    /*.change_virtual_intf = */woal_cfg80211_change_virtual_intf,
    /*.add_key = */woal_cfg80211_add_key,
    /*.get_key = ,*/
    /*.del_key = */woal_cfg80211_del_key,
    /*.set_default_key = */woal_cfg80211_set_default_key,
    /*.set_default_mgmt_key = 0,*/
	/*.start_ap = */woal_cfg80211_add_beacon,
    /*.change_beacon = */woal_cfg80211_set_beacon,
    /*.stop_ap = */woal_cfg80211_del_beacon,
	/*.add_station = 0,
	.del_station = 0,
	.change_station = 0,*/
	/*.get_station = */woal_cfg80211_get_station,
	/*.dump_station = */woal_cfg80211_dump_station,
	/*.add_mpath = 0,
	.del_mpath = 0,
	.change_mpath = 0,
	.get_mpath = 0,
	.dump_mpath = 0,
	.get_mesh_config = 0,
	.update_mesh_config = 0,
	.join_mesh = 0,
	.leave_mesh = 0,
	.change_bss = 0,
	.set_txq_params = 0,
	.libertas_set_mesh_channel = 0,
	.set_monitor_channel = 0,*/
    /*.scan = */woal_cfg80211_scan,
    /*.auth = 0,
	.assoc = 0,
	.deauth = 0,
	.disassoc = 0,*/
   /* .connect = */woal_cfg80211_connect,
    /*.disconnect = */woal_cfg80211_disconnect,
	/*.join_ibss = */0,
    /*.leave_ibss = */0,
	/*.set_mcast_rate = 0,*/
    /*.set_wiphy_params = */woal_cfg80211_set_wiphy_params,
    /*.set_tx_power = */woal_cfg80211_set_tx_power,
    /*.get_tx_power = 0,
    .set_wds_peer = 0,
    .rfkill_poll = 0,
    .set_bitrate_mask = 0,*/

	  /*.mgmt_tx = */woal_cfg80211_mgmt_tx,
	  /*.mgmt_tx_cancel_wait = */woal_cfg80211_mgmt_tx_cancel_wait,
    /*.remain_on_channel = */woal_cfg80211_remain_on_channel,
    /*.cancel_remain_on_channel = */woal_cfg80211_cancel_remain_on_channel,
    /*.set_power_mgmt = */woal_cfg80211_set_power_mgmt,
    /*.set_cqm_rssi_config = */0,
    
    
    /*.mgmt_frame_register = */woal_cfg80211_mgmt_frame_register,
    /*.set_channel = */woal_cfg80211_set_channel,
/*

    .change_virtual_intf = ,
    .scan = ,
    .connect = ,
    .disconnect = ,
    .get_station = ,
    .dump_station = ,
    .set_wiphy_params = ,
    .set_channel = ,
    .add_key = ,
    .del_key = ,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
    .start_ap = ,
    .change_beacon = ,
    .stop_ap = ,
#else
    .add_beacon = woal_cfg80211_add_beacon,
    .set_beacon = woal_cfg80211_set_beacon,
    .del_beacon = woal_cfg80211_del_beacon,
#endif
    .mgmt_frame_register = ,
    .mgmt_tx = ,
    .mgmt_tx_cancel_wait = woal_cfg80211_mgmt_tx_cancel_wait,
    .remain_on_channel = woal_cfg80211_remain_on_channel,
    .cancel_remain_on_channel = woal_cfg80211_cancel_remain_on_channel,
    .set_default_key = ,
    .set_power_mgmt = ,
    .set_tx_power = ,*/
};
#endif /* KERNEL_VERSION */
#endif /* WIFI_DIRECT_SUPPORT && V14_FEATURE */


/********************************************************
				Local Variables
********************************************************/
#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
static const struct ieee80211_txrx_stypes
    ieee80211_mgmt_stypes[NUM_NL80211_IFTYPES] = {
    /*[NL80211_IFTYPE_UNSPECIFIED] = */{
                              /*.tx = */0x0000,
                              /*.rx = */0x0000,
                              },
    /*[NL80211_IFTYPE_ADHOC] = */{
                              /*.tx = */0x0000,
                              /*.rx = */0x0000,
                              },
    /*[NL80211_IFTYPE_STATION] = */{
                                /*.tx = */BIT(IEEE80211_STYPE_ACTION >> 4) |
                                BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
                                /*.rx = */BIT(IEEE80211_STYPE_ACTION >> 4) |
                                BIT(IEEE80211_STYPE_PROBE_REQ >> 4),
                                },
    /*[NL80211_IFTYPE_AP] = */{
                           /*.tx = */0xffff,
                           /*.rx = */BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
                           BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
                           BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
                           BIT(IEEE80211_STYPE_DISASSOC >> 4) |
                           BIT(IEEE80211_STYPE_AUTH >> 4) |
                           BIT(IEEE80211_STYPE_DEAUTH >> 4) |
                           BIT(IEEE80211_STYPE_ACTION >> 4),
                           },
    /*[NL80211_IFTYPE_AP_VLAN] = */{
                                /*.tx = */0x0000,
                                /*.rx = */0x0000,
                                },
    /*[NL80211_IFTYPE_WDS] = */{
                                /*.tx = */0x0000,
                                /*.rx = */0x0000,
                                },
    /*[NL80211_IFTYPE_MONITOR] = */{
                                /*.tx = */0x0000,
                                /*.rx = */0x0000,
                                },
    /*[NL80211_IFTYPE_MESH_POINT] = */{
                                   /*.tx = */0x0000,
                                   /*.rx = */0x0000,
                                   },                           
    /*[NL80211_IFTYPE_P2P_CLIENT] = */{
                                   /*.tx = */BIT(IEEE80211_STYPE_ACTION >> 4) |
                                   BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
                                   /*.rx = */BIT(IEEE80211_STYPE_ACTION >> 4) |
                                   BIT(IEEE80211_STYPE_PROBE_REQ >> 4),
                                   },
    /*[NL80211_IFTYPE_P2P_GO] = */{
                               /*.tx = */BIT(IEEE80211_STYPE_ACTION >> 4) |
                               // BIT(IEEE80211_STYPE_PROBE_RESP >> 4) |
                               0,
                               /*.rx = */BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
                               BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
                               BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
                               BIT(IEEE80211_STYPE_DISASSOC >> 4) |
                               BIT(IEEE80211_STYPE_AUTH >> 4) |
                               BIT(IEEE80211_STYPE_DEAUTH >> 4) |
                               BIT(IEEE80211_STYPE_ACTION >> 4),
                               },
    

};
#endif
#endif /* WIFI_DIRECT_SUPPORT && V14_FEATURE */

/********************************************************
				Global Variables
********************************************************/

/********************************************************
				Local Functions
********************************************************/
/**
 * @brief Get the encryption mode from cipher
 *
 * @param cipher        Cipher cuite
 * @param wpa_enabled   WPA enable or disable
 *
 * @return              MLAN_ENCRYPTION_MODE_*
 */
static int
woal_cfg80211_get_encryption_mode(t_u32 cipher, int *wpa_enabled)
{
    int encrypt_mode;

    ENTER();

    *wpa_enabled = 0;
    switch (cipher) {
    case IW_AUTH_CIPHER_NONE:
        encrypt_mode = MLAN_ENCRYPTION_MODE_NONE;
        break;
    case WLAN_CIPHER_SUITE_WEP40:
        encrypt_mode = MLAN_ENCRYPTION_MODE_WEP40;
        break;
    case WLAN_CIPHER_SUITE_WEP104:
        encrypt_mode = MLAN_ENCRYPTION_MODE_WEP104;
        break;
    case WLAN_CIPHER_SUITE_TKIP:
        encrypt_mode = MLAN_ENCRYPTION_MODE_TKIP;
        *wpa_enabled = 1;
        break;
    case WLAN_CIPHER_SUITE_CCMP:
        encrypt_mode = MLAN_ENCRYPTION_MODE_CCMP;
        *wpa_enabled = 1;
        break;
    default:
        encrypt_mode = -1;
    }

    LEAVE();
    return encrypt_mode;
}

/**
 *  @brief Check the pairwise or group cipher for
 *  WEP enabled or not
 *
 *  @param cipher       MLAN Cipher cuite
 *
 *  @return             1 -- enable or 0 -- disable
 */
static int
woal_cfg80211_is_alg_wep(t_u32 cipher)
{
    int alg = 0;
    ENTER();

    if (cipher == MLAN_ENCRYPTION_MODE_WEP40 ||
        cipher == MLAN_ENCRYPTION_MODE_WEP104)
        alg = 1;

    LEAVE();
    return alg;
}

/**
 *  @brief Convert NL802.11 channel type into driver channel type
 *
 * The mapping is as follows -
 *      NL80211_CHAN_NO_HT     -> NO_SEC_CHANNEL
 *      NL80211_CHAN_HT20      -> NO_SEC_CHANNEL
 *      NL80211_CHAN_HT40PLUS  -> SEC_CHANNEL_ABOVE
 *      NL80211_CHAN_HT40MINUS -> SEC_CHANNEL_BELOW
 *      Others                 -> NO_SEC_CHANNEL
 *
 *  @param channel_type     Channel type
 *
 *  @return                 Driver channel type
 */
static int
woal_cfg80211_channel_type_to_channel(enum nl80211_channel_type channel_type)
{
    int channel;

    ENTER();

    switch (channel_type) {
    case NL80211_CHAN_NO_HT:
    case NL80211_CHAN_HT20:
        channel = NO_SEC_CHANNEL;
        break;
    case NL80211_CHAN_HT40PLUS:
        channel = SEC_CHANNEL_ABOVE;
        break;
    case NL80211_CHAN_HT40MINUS:
        channel = SEC_CHANNEL_BELOW;
        break;
    default:
        channel = NO_SEC_CHANNEL;
    }
    LEAVE();
    return channel;
}

/**
 *  @brief Convert secondary channel type to NL80211 channel type
 *
 * The mapping is as follows -
 *      NO_SEC_CHANNEL      -> NL80211_CHAN_HT20
 *      SEC_CHANNEL_ABOVE   -> NL80211_CHAN_HT40PLUS
 *      SEC_CHANNEL_BELOW   -> NL80211_CHAN_HT40MINUS
 *      Others              -> NL80211_CHAN_HT20
 *
 *  @param channel_type     Driver channel type
 *
 *  @return                 nl80211_channel_type type
 */
static enum nl80211_channel_type
woal_channel_to_nl80211_channel_type(int channel_type)
{
    enum nl80211_channel_type channel;

    ENTER();

    switch (channel_type) {
    case NO_SEC_CHANNEL:
        channel = NL80211_CHAN_HT20;
        break;
    case SEC_CHANNEL_ABOVE:
        channel = NL80211_CHAN_HT40PLUS;
        break;
    case SEC_CHANNEL_BELOW:
        channel = NL80211_CHAN_HT40MINUS;
        break;
    default:
        channel = NL80211_CHAN_HT20;
    }
    LEAVE();
    return channel;
}

/**
 *  @brief Convert driver band configuration to IEEE band type
 *
 *  @param band     Driver band configuration
 *
 *  @return         IEEE band type
 */
enum ieee80211_band
woal_band_cfg_to_ieee_band(t_u32 band)
{
    enum ieee80211_band ret_radio_type;

    ENTER();

    switch (band) {
    case BAND_A:
    case BAND_AN:
    case BAND_A | BAND_AN:
        ret_radio_type = IEEE80211_BAND_5GHZ;
        break;
    case BAND_B:
    case BAND_G:
    case BAND_B | BAND_G:
    case BAND_GN:
    case BAND_B | BAND_GN:
    default:
        ret_radio_type = IEEE80211_BAND_2GHZ;
        break;
    }

    LEAVE();
    return ret_radio_type;
}

/**
 *  @brief Convert NL80211 interface type to MLAN_BSS_MODE_*
 *
 *  @param iftype   Interface type of NL80211
 *
 *  @return         Driver bss mode
 */
static t_u32
woal_nl80211_iftype_to_mode(enum nl80211_iftype iftype)
{
    switch (iftype) {
    case NL80211_IFTYPE_ADHOC:
        return MLAN_BSS_MODE_IBSS;
    case NL80211_IFTYPE_STATION:
        return MLAN_BSS_MODE_INFRA;
    case NL80211_IFTYPE_UNSPECIFIED:
    default:
        return MLAN_BSS_MODE_AUTO;
    }
}

/**
 *  @brief Control WPS Session Enable/Disable
 *
 *  @param priv     Pointer to the moal_private driver data struct
 *  @param enable   enable/disable flag
 *
 *  @return          0 --success, otherwise fail
 */
static int
woal_wps_cfg(moal_private * priv, int enable)
{
    int ret = 0;
    mlan_ds_wps_cfg *pwps = NULL;
    mlan_ioctl_req *req = NULL;

    ENTER();

    PRINTM(MINFO, "WOAL_WPS_SESSION\n");

    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_wps_cfg));
    if (req == NULL) {
        ret = -ENOMEM;
        goto done;
    }

    pwps = (mlan_ds_wps_cfg *) req->pbuf;
    req->req_id = MLAN_IOCTL_WPS_CFG;
    req->action = MLAN_ACT_SET;
    pwps->sub_command = MLAN_OID_WPS_CFG_SESSION;
    if (enable)
        pwps->param.wps_session = MLAN_WPS_CFG_SESSION_START;
    else
        pwps->param.wps_session = MLAN_WPS_CFG_SESSION_END;

    if (MLAN_STATUS_SUCCESS != woal_request_ioctl(priv, req, MOAL_IOCTL_WAIT)) {
        ret = -EFAULT;
        goto done;
    }

  done:
    if (req)
        kfree(req);
    LEAVE();
    return ret;
}

/**
 * @brief configure ASSOC IE 
 *
 * @param priv				A pointer to moal private structure
 * @param ie				A pointer to ie data
 * @param ie_len			The length of ie data
 *
 * @return                  0 -- success, otherwise fail
 */
static int
woal_cfg80211_assoc_ies_cfg(moal_private * priv, t_u8 * ie, int ie_len)
{
    int bytes_left = ie_len;
    t_u8 *pcurrent_ptr = ie;
    int total_ie_len;
    t_u8 element_len;
    int ret = MLAN_STATUS_SUCCESS;
    IEEEtypes_ElementId_e element_id;
    IEEEtypes_VendorSpecific_t *pvendor_ie;
    t_u8 wps_oui[] = { 0x00, 0x50, 0xf2, 0x04 };

    while (bytes_left >= 2) {
        element_id = (IEEEtypes_ElementId_e) (*((t_u8 *) pcurrent_ptr));
        element_len = *((t_u8 *) pcurrent_ptr + 1);
        total_ie_len = element_len + sizeof(IEEEtypes_Header_t);
        if (bytes_left < total_ie_len) {
            PRINTM(MERROR, "InterpretIE: Error in processing IE, "
                   "bytes left < IE length\n");
            bytes_left = 0;
            continue;
        }
        switch (element_id) {
        case RSN_IE:
            if (MLAN_STATUS_SUCCESS !=
                woal_set_get_gen_ie(priv, MLAN_ACT_SET, pcurrent_ptr,
                                    &total_ie_len)) {
                PRINTM(MERROR, "Fail to set RSN IE\n");
                ret = -EFAULT;
                goto done;
            }
            PRINTM(MIOCTL, "Set RSN IE\n");
            break;
        case VENDOR_SPECIFIC_221:
            pvendor_ie = (IEEEtypes_VendorSpecific_t *) pcurrent_ptr;
            if (!memcmp(pvendor_ie->vend_hdr.oui, wps_oui, sizeof(wps_oui))) {
                PRINTM(MIOCTL, "Enable WPS session\n");
                woal_wps_cfg(priv, MTRUE);
            }
            if (MLAN_STATUS_SUCCESS !=
                woal_set_get_gen_ie(priv, MLAN_ACT_SET, pcurrent_ptr,
                                    &total_ie_len)) {
                PRINTM(MERROR, "Fail to Set VENDOR SPECIFIC IE\n");
                ret = -EFAULT;
                goto done;
            }
            PRINTM(MIOCTL, "Set VENDOR SPECIFIC IE, OUI: %02x:%02x:%02x:%02x\n",
                   pvendor_ie->vend_hdr.oui[0], pvendor_ie->vend_hdr.oui[1],
                   pvendor_ie->vend_hdr.oui[2], pvendor_ie->vend_hdr.oui_type);
            break;
        default:
            if (MLAN_STATUS_SUCCESS !=
                woal_set_get_gen_ie(priv, MLAN_ACT_SET, pcurrent_ptr,
                                    &total_ie_len)) {
                PRINTM(MERROR, "Fail to set GEN IE\n");
                ret = -EFAULT;
                goto done;
            }
            PRINTM(MIOCTL, "Set GEN IE\n");
            break;
        }
        pcurrent_ptr += element_len + 2;
        /* Need to account for IE ID and IE Len */
        bytes_left -= (element_len + 2);
    }
  done:
    return ret;
}

/**
 * @brief Send domain info command to FW
 *
 * @param priv      A pointer to moal_private structure
 *
 * @return          MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
static mlan_status
woal_send_domain_info_cmd_fw(moal_private * priv)
{
    mlan_status ret = MLAN_STATUS_SUCCESS;
    enum ieee80211_band band;
    struct ieee80211_supported_band *sband = NULL;
    struct ieee80211_channel *channel = NULL;
    t_u8 no_of_sub_band = 0;
    t_u8 no_of_parsed_chan = 0;
    t_u8 first_chan = 0, next_chan = 0, max_pwr = 0;
    t_u8 i, flag = 0;
    mlan_ds_11d_cfg *cfg_11d = NULL;
    mlan_ds_radio_cfg *radio_cfg = NULL;
    mlan_ioctl_req *req = NULL;

    ENTER();

    if (!priv->wdev || !priv->wdev->wiphy) {
        PRINTM(MERROR, "No wdev or wiphy in priv\n");
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }

    /* Allocate an IOCTL request buffer */
    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_radio_cfg));
    if (req == NULL) {
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }

    radio_cfg = (mlan_ds_radio_cfg *) req->pbuf;
    radio_cfg->sub_command = MLAN_OID_BAND_CFG;
    req->req_id = MLAN_IOCTL_RADIO_CFG;
    req->action = MLAN_ACT_GET;
    if (MLAN_STATUS_SUCCESS != woal_request_ioctl(priv, req, MOAL_IOCTL_WAIT)) {
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }
    band = woal_band_cfg_to_ieee_band(radio_cfg->param.band_cfg.config_bands);
    if (!priv->wdev->wiphy->bands[band]) {
        PRINTM(MERROR, "11D: setting domain info in FW failed");
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }
    kfree(req);
    req = NULL;

    /* Allocate an IOCTL request buffer */
    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_11d_cfg));
    if (req == NULL) {
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }
    cfg_11d = (mlan_ds_11d_cfg *) req->pbuf;
    cfg_11d->sub_command = MLAN_OID_11D_DOMAIN_INFO;
    req->req_id = MLAN_IOCTL_11D_CFG;
    req->action = MLAN_ACT_SET;

    /* Set country code */
    cfg_11d->param.domain_info.country_code[0] = priv->country_code[0];
    cfg_11d->param.domain_info.country_code[1] = priv->country_code[1];
    cfg_11d->param.domain_info.country_code[2] = ' ';
    cfg_11d->param.domain_info.band = band;

    sband = priv->wdev->wiphy->bands[band];
    for (i = 0; (i < sband->n_channels) &&
         (no_of_sub_band < MRVDRV_MAX_SUBBAND_802_11D); i++) {
        channel = &sband->channels[i];
        if (channel->flags & IEEE80211_CHAN_DISABLED)
            continue;

        if (!flag) {
            flag = 1;
            next_chan = first_chan = (t_u32) channel->hw_value;
            max_pwr = channel->max_power;
            no_of_parsed_chan = 1;
            continue;
        }

        if (channel->hw_value == next_chan + 1 && channel->max_power == max_pwr) {
            next_chan++;
            no_of_parsed_chan++;
        } else {
            cfg_11d->param.domain_info.sub_band[no_of_sub_band]
                .first_chan = first_chan;
            cfg_11d->param.domain_info.sub_band[no_of_sub_band]
                .no_of_chan = no_of_parsed_chan;
            cfg_11d->param.domain_info.sub_band[no_of_sub_band]
                .max_tx_pwr = max_pwr;
            no_of_sub_band++;
            next_chan = first_chan = (t_u32) channel->hw_value;
            max_pwr = channel->max_power;
            no_of_parsed_chan = 1;
        }
    }

    if (flag) {
        cfg_11d->param.domain_info.sub_band[no_of_sub_band]
            .first_chan = first_chan;
        cfg_11d->param.domain_info.sub_band[no_of_sub_band]
            .no_of_chan = no_of_parsed_chan;
        cfg_11d->param.domain_info.sub_band[no_of_sub_band]
            .max_tx_pwr = max_pwr;
        no_of_sub_band++;
    }
    cfg_11d->param.domain_info.no_of_sub_band = no_of_sub_band;

    /* Send domain info command to FW */
    if (MLAN_STATUS_SUCCESS != woal_request_ioctl(priv, req, MOAL_IOCTL_WAIT)) {
        ret = MLAN_STATUS_FAILURE;
        PRINTM(MERROR, "11D: Error setting domain info in FW\n");
        goto done;
    }

  done:
    if (req)
        kfree(req);
    LEAVE();
    return ret;
}

/**
 * @brief Request the driver to change the channel and
 * change domain info according to that channel
 *
 * @param priv            A pointer to moal_private structure
 * @param chan            A pointer to ieee80211_channel structure
 * @param channel_type    Channel type of nl80211_channel_type
 *
 * @return                0 -- success, otherwise fail
 */
int
woal_set_rf_channel(moal_private * priv,
                    struct ieee80211_channel *chan,
                    enum nl80211_channel_type channel_type)
{
    int ret = 0;
    t_u32 mode, config_bands = 0;
    mlan_ioctl_req *req = NULL;
    mlan_ds_radio_cfg *radio_cfg = NULL;

    ENTER();

    if (!chan) {
        LEAVE();
        return EINVAL;
    }
    mode = woal_nl80211_iftype_to_mode(priv->wdev->iftype);
    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_radio_cfg));
    if (req == NULL) {
        ret = -ENOMEM;
        goto done;
    }
    radio_cfg = (mlan_ds_radio_cfg *) req->pbuf;
    radio_cfg->sub_command = MLAN_OID_BAND_CFG;
    req->req_id = MLAN_IOCTL_RADIO_CFG;
    /* Get config_bands, adhoc_start_band and adhoc_channel values from MLAN */
    req->action = MLAN_ACT_GET;
    if (MLAN_STATUS_SUCCESS != woal_request_ioctl(priv, req, MOAL_IOCTL_WAIT)) {
        ret = -EFAULT;
        goto done;
    }
    if (chan) {
        req->action = MLAN_ACT_SET;
        /* Set appropriate bands */
        if (chan->band == IEEE80211_BAND_2GHZ)
            config_bands = BAND_B | BAND_G | BAND_GN;
        else
            config_bands = BAND_AN | BAND_A;
        if (mode == MLAN_BSS_MODE_IBSS) {
            radio_cfg->param.band_cfg.adhoc_start_band = config_bands;
            radio_cfg->param.band_cfg.adhoc_channel =
                ieee80211_frequency_to_channel(chan->center_freq);
        }
        /* Set channel offset */
        radio_cfg->param.band_cfg.sec_chan_offset =
            woal_cfg80211_channel_type_to_channel(channel_type);

        if (MLAN_STATUS_SUCCESS !=
            woal_request_ioctl(priv, req, MOAL_IOCTL_WAIT)) {
            ret = -EFAULT;
            goto done;
        }
        woal_send_domain_info_cmd_fw(priv);
    }

    PRINTM(MINFO,
           "Setting band %d, channel bandwidth %d and mode = %d channel=%d\n",
           config_bands, radio_cfg->param.band_cfg.sec_chan_offset, mode,
           ieee80211_frequency_to_channel(chan->center_freq));

    if (MLAN_STATUS_SUCCESS !=
        woal_change_adhoc_chan(priv,
                               ieee80211_frequency_to_channel(chan->
                                                              center_freq))) {
        ret = -EFAULT;
        goto done;
    }

  done:
    if (req)
        kfree(req);
    LEAVE();
    return ret;
}

/** 
 *  @brief Set ewpa mode
 *
 *  @param priv                 A pointer to moal_private structure
 *  @param wait_option          Wait option
 *  @param ssid_bssid           A pointer to mlan_ssid_bssid structure
 *
 *  @return                     MLAN_STATUS_SUCCESS/MLAN_STATUS_PENDING -- success, otherwise fail          
 */
mlan_status
woal_set_ewpa_mode(moal_private * priv, t_u8 wait_option,
                   mlan_ssid_bssid * ssid_bssid, bool enable)
{
    mlan_ioctl_req *req = NULL;
    mlan_ds_sec_cfg *sec = NULL;
    mlan_status status = MLAN_STATUS_SUCCESS;

    ENTER();

    /* Allocate an IOCTL request buffer */
    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_sec_cfg));
    if (req == NULL) {
        status = MLAN_STATUS_FAILURE;
        goto error;
    }
    /* Fill request buffer */
    sec = (mlan_ds_sec_cfg *) req->pbuf;
    sec->sub_command = MLAN_OID_SEC_CFG_PASSPHRASE;
    req->req_id = MLAN_IOCTL_SEC_CFG;
    req->action = MLAN_ACT_GET;
#if 0 //edit by forrest 无法获取到passphrase??
    /* Try Get All */
    memset(&sec->param.passphrase, 0, sizeof(mlan_ds_passphrase));
    memcpy(&sec->param.passphrase.ssid, &ssid_bssid->ssid,
           sizeof(sec->param.passphrase.ssid));
    memcpy(&sec->param.passphrase.bssid, &ssid_bssid->bssid,
           MLAN_MAC_ADDR_LENGTH);
    sec->param.passphrase.psk_type = MLAN_PSK_QUERY;

    /* Send IOCTL request to MLAN */
    status = woal_request_ioctl(priv, req, wait_option);
    if (status != MLAN_STATUS_SUCCESS)
        goto error;
    sec->param.ewpa_enabled = MFALSE;
    if (sec->param.passphrase.psk_type == MLAN_PSK_PASSPHRASE) {
        if (sec->param.passphrase.psk.passphrase.passphrase_len > 0) {
            sec->param.ewpa_enabled = MTRUE;
        }
    } else if (sec->param.passphrase.psk_type == MLAN_PSK_PMK)
        sec->param.ewpa_enabled = MTRUE;
#endif

	if(enable)
		sec->param.ewpa_enabled = MTRUE;
	else
		sec->param.ewpa_enabled = MFALSE;
	
    sec->sub_command = MLAN_OID_SEC_CFG_EWPA_ENABLED;
    req->action = MLAN_ACT_SET;

    /* Send IOCTL request to MLAN */
    status = woal_request_ioctl(priv, req, wait_option);

  error:
    if (req && (status != MLAN_STATUS_PENDING))
        kfree(req);
    LEAVE();
    return status;
}


/** 
 *  @brief add by forrest
 *
 *  @param priv                 A pointer to moal_private structure
 *  @param wait_option          Wait option
 *  @param ssid_bssid           A pointer to mlan_ssid_bssid structure
 *
 *  @return                     MLAN_STATUS_SUCCESS/MLAN_STATUS_PENDING -- success, otherwise fail          
 */
mlan_status
woal_set_passphrase(moal_private * priv, t_u8 wait_option,
                   mlan_ds_passphrase * passphrase)
{
    mlan_ioctl_req *req = NULL;
    mlan_ds_sec_cfg *sec = NULL;
    mlan_status status = MLAN_STATUS_SUCCESS;

    ENTER();

    /* Allocate an IOCTL request buffer */
    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_sec_cfg));
    if (req == NULL) {
        status = MLAN_STATUS_FAILURE;
        goto error;
    }
    /* Fill request buffer */
    sec = (mlan_ds_sec_cfg *) req->pbuf;
    sec->sub_command = MLAN_OID_SEC_CFG_PASSPHRASE;
    req->req_id = MLAN_IOCTL_SEC_CFG;
    req->action = MLAN_ACT_SET;

    /* Try Get All */
    memcpy(&sec->param.passphrase, passphrase, sizeof(mlan_ds_passphrase));

    /* Send IOCTL request to MLAN */
    status = woal_request_ioctl(priv, req, wait_option);
    if (status != MLAN_STATUS_SUCCESS)
        goto error;
    
  error:
    if (req && (status != MLAN_STATUS_PENDING))
        kfree(req);
    LEAVE();
    return status;
}


/**
 * @brief Set encryption mode and enable WPA
 *
 * @param priv          A pointer to moal_private structure
 * @param encrypt_mode  Encryption mode
 * @param wpa_enabled   WPA enable or not
 *
 * @return              0 -- success, otherwise fail
 */
static int
woal_cfg80211_set_auth(moal_private * priv, int encrypt_mode, int wpa_enabled)
{
    int ret = 0;

    ENTER();

    if (MLAN_STATUS_SUCCESS !=
        woal_set_encrypt_mode(priv, MOAL_IOCTL_WAIT, encrypt_mode))
        ret = -EFAULT;

    if (wpa_enabled) {
        if (MLAN_STATUS_SUCCESS !=
            woal_set_wpa_enable(priv, MOAL_IOCTL_WAIT, 1))
            ret = -EFAULT;
    }

    LEAVE();
    return ret;
}

/**
 * @brief Informs the CFG802.11 subsystem of a new BSS connection.
 *
 * The following information are sent to the CFG802.11 subsystem
 * to register the new BSS connection. If we do not register the new BSS,
 * a kernel panic will result.
 *      - MAC address
 *      - Capabilities
 *      - Beacon period
 *      - RSSI value
 *      - Channel
 *      - Supported rates IE
 *      - Extended capabilities IE
 *      - DS parameter set IE
 *      - HT Capability IE
 *      - Vendor Specific IE (221)
 *      - WPA IE
 *      - RSN IE
 *
 * @param priv      A pointer to moal_private structure
 * @param ssid      A pointer to mlan_802_11_ssid structure
 *
 * @return          MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
extern void cfg80211_put_bss(struct cfg80211_bss *pub);
mlan_status
woal_inform_bss_from_scan_result(moal_private * priv, mlan_802_11_ssid * ssid)
{
    mlan_status ret = MLAN_STATUS_SUCCESS;
    struct ieee80211_channel *chan;
    mlan_scan_resp scan_resp;
    BSSDescriptor_t *scan_table;
    t_u64 ts = 0;
    u16 cap_info = 0;
    int i = 0;
    struct cfg80211_bss *pub = NULL;
    ENTER();
    if (!priv->wdev || !priv->wdev->wiphy) {
        LEAVE();
        return MLAN_STATUS_FAILURE;
    }

    memset(&scan_resp, 0, sizeof(scan_resp));
    if (MLAN_STATUS_SUCCESS != woal_get_scan_table(priv,
                                                   MOAL_IOCTL_WAIT,
                                                   &scan_resp)) {
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }

    if (scan_resp.num_in_scan_table) {
        scan_table = (BSSDescriptor_t *) scan_resp.pscan_table;
        for (i = 0; i < scan_resp.num_in_scan_table; i++) {
            if (ssid) {
                /* Inform specific BSS only */
                if (memcmp(ssid->ssid, scan_table[i].ssid.ssid, ssid->ssid_len))
                    continue;
            }
            if (!scan_table[i].freq) {
                PRINTM(MERROR, "Invalid channel number %d\n",
                       (int) scan_table[i].channel);
                continue;
            }
            chan = ieee80211_get_channel(priv->wdev->wiphy, scan_table[i].freq);
            if (!chan) {
                PRINTM(MERROR,
                       "Fail to get chan with freq: channel=%d freq=%d\n",
                       (int) scan_table[i].channel, (int) scan_table[i].freq);
                continue;
            }
            memcpy(&ts, scan_table[i].time_stamp, sizeof(ts));
            memcpy(&cap_info, &scan_table[i].cap_info, sizeof(cap_info));
            pub = cfg80211_inform_bss(priv->wdev->wiphy, chan,
                                      scan_table[i].mac_address,
                                      scan_table[i].ssid.ssid,
                                      scan_table[i].ssid.ssid_len,
                                      ts, cap_info,
                                      scan_table[i].beacon_period,
                                      scan_table[i].pbeacon_buf +
                                      WLAN_802_11_FIXED_IE_SIZE,
                                      scan_table[i].beacon_buf_size -
                                      WLAN_802_11_FIXED_IE_SIZE,
                                      /*-RSSI_DBM_TO_MDM*/(scan_table[i].rssi),
                                      GFP_KERNEL);
            if (pub) {
                pub->len_information_elements = pub->len_beacon_ies;
                cfg80211_put_bss(pub);
            }
        }
    }
  done:
    LEAVE();
    return ret;
}

/**
 * @brief Informs the CFG802.11 subsystem of a new IBSS connection.
 *
 * The following information are sent to the CFG802.11 subsystem
 * to register the new IBSS connection. If we do not register the
 * new IBSS, a kernel panic will result.
 *      - MAC address
 *      - Capabilities
 *      - Beacon period
 *      - RSSI value
 *      - Channel
 *      - Supported rates IE
 *      - Extended capabilities IE
 *      - DS parameter set IE
 *      - HT Capability IE
 *      - Vendor Specific IE (221)
 *      - WPA IE
 *      - RSN IE
 *
 * @param priv              A pointer to moal_private structure
 * @param cahn              A pointer to ieee80211_channel structure
 * @param beacon_interval   Beacon interval
 *
 * @return                  MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
static mlan_status
woal_cfg80211_inform_ibss_bss(moal_private * priv,
                              struct ieee80211_channel *chan,
                              t_u16 beacon_interval)
{
    mlan_status ret = MLAN_STATUS_SUCCESS;
    mlan_bss_info bss_info;
    mlan_ds_get_signal signal;
    t_u8 ie_buf[MLAN_MAX_SSID_LENGTH + sizeof(IEEEtypes_Header_t)];
    int ie_len = 0;
    struct cfg80211_bss *bss = NULL;

    ENTER();

    ret = woal_get_bss_info(priv, MOAL_IOCTL_WAIT, &bss_info);
    if (ret)
        goto done;

    memset(ie_buf, 0, sizeof(ie_buf));
    ie_buf[0] = WLAN_EID_SSID;
    ie_buf[1] = bss_info.ssid.ssid_len;

    memcpy(&ie_buf[sizeof(IEEEtypes_Header_t)],
           &bss_info.ssid.ssid, bss_info.ssid.ssid_len);
    ie_len = ie_buf[1] + sizeof(IEEEtypes_Header_t);

    /* Get signal information from the firmware */
    memset(&signal, 0, sizeof(mlan_ds_get_signal));
    if (MLAN_STATUS_SUCCESS !=
        woal_get_signal_info(priv, MOAL_IOCTL_WAIT, &signal)) {
        PRINTM(MERROR, "Error getting signal information\n");
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }

    bss = cfg80211_inform_bss(priv->wdev->wiphy, chan,
                              bss_info.bssid, 
							  bss_info.ssid.ssid,
							  bss_info.ssid.ssid_len,
							  0, WLAN_CAPABILITY_IBSS,
                              beacon_interval, ie_buf, ie_len,
                              signal.bcn_rssi_avg, GFP_KERNEL);
    if (bss)
        cfg80211_put_bss(bss);
  done:
    LEAVE();
    return ret;
}

/**
 * @brief Request the driver for (re)association
 *
 * @param priv            A pointer to moal_private structure
 * @param sme             A pointer to connect parameters
 *
 * @return                0 -- success, otherwise fail
 */
extern struct scan_result_data lastest_scan_result;
static int
woal_cfg80211_assoc(moal_private * priv, void *sme)
{
    struct cfg80211_ibss_params *ibss_param = NULL;
    struct cfg80211_connect_params *conn_param = NULL;
    mlan_802_11_ssid req_ssid;
    mlan_ssid_bssid ssid_bssid;
    mlan_ds_radio_cfg *radio_cfg = NULL;
    mlan_ioctl_req *req = NULL;
	mlan_ds_passphrase *passphrase = NULL;
    int ret = 0;
    t_u32 auth_type = 0, mode;
    int wpa_enabled = 0;
    int group_enc_mode = 0, pairwise_enc_mode = 0;
    int alg_is_wep = 0;

    t_u8 *ssid, ssid_len = 0, *bssid;
    t_u8 *ie = NULL;
    int ie_len = 0;
    struct ieee80211_channel *channel = NULL;
    t_u16 beacon_interval = 0;
    bool privacy;

    ENTER();

    mode = woal_nl80211_iftype_to_mode(priv->wdev->iftype);

    if (mode == MLAN_BSS_MODE_IBSS) {
        ibss_param = (struct cfg80211_ibss_params *) sme;
        ssid = ibss_param->ssid;
        ssid_len = ibss_param->ssid_len;
        bssid = ibss_param->bssid;
        channel = ibss_param->channel;
        if (ibss_param->ie_len)
            ie = ibss_param->ie;
        ie_len = ibss_param->ie_len;
        beacon_interval = ibss_param->beacon_interval;
        privacy = ibss_param->privacy;

#ifdef REASSOCIATION	
		if(ibss_param->key && ibss_param->key_len){
			memcpy(priv->prev_passphrase.passphrase, ibss_param->key, MLAN_MAX_PASSPHRASE_LENGTH);
			priv->prev_passphrase.passphrase_len = ibss_param->key_len;
		}else{

			priv->prev_passphrase.passphrase_len = 0;
		}
#endif	

    } else {
        conn_param = (struct cfg80211_connect_params *) sme;
        ssid = conn_param->ssid;
        ssid_len = conn_param->ssid_len;
        bssid = conn_param->bssid;
        channel = conn_param->channel;
        if (conn_param->ie_len)
            ie = conn_param->ie;
        ie_len = conn_param->ie_len;
        privacy = conn_param->privacy;
#ifdef REASSOCIATION	
		if(conn_param->key_len){
			memcpy(priv->prev_passphrase.passphrase, conn_param->key, MLAN_MAX_PASSPHRASE_LENGTH);
			priv->prev_passphrase.passphrase_len = conn_param->key_len;
		}else{
		
			priv->prev_passphrase.passphrase_len = 0;
		}
#endif

    }

    memset(&req_ssid, 0, sizeof(mlan_802_11_ssid));
    memset(&ssid_bssid, 0, sizeof(mlan_ssid_bssid));

    req_ssid.ssid_len = ssid_len;
    if (ssid_len > IW_ESSID_MAX_SIZE) {
        PRINTM(MERROR, "Invalid SSID - aborting\n");
        ret = -EINVAL;
        goto done;
    }

    memcpy(req_ssid.ssid, ssid, ssid_len);
    if (!req_ssid.ssid_len || req_ssid.ssid[0] < 0x20) {
        PRINTM(MERROR, "Invalid SSID - aborting\n");
        ret = -EINVAL;
        goto done;
    }

#if 1	//add by forrest 根据搜到的ap信息填充完善conn_param结构
	if (mode == MLAN_BSS_MODE_INFRA)
	{
		memcpy(&ssid_bssid.ssid, &req_ssid, sizeof(mlan_802_11_ssid));
	    if (bssid)
	        memcpy(&ssid_bssid.bssid, bssid, ETH_ALEN);
	    if (MLAN_STATUS_SUCCESS != woal_find_essid(priv, &ssid_bssid)) {
	        /* Do specific SSID scanning */
	        if (MLAN_STATUS_SUCCESS !=
	            woal_request_scan(priv, MOAL_IOCTL_WAIT, &req_ssid)) {
	            ret = -EFAULT;
	            goto done;
	        }
	    }

		if(strcmp((char*)ssid, (char*)lastest_scan_result.essid) == 0)
		{
			//conn_param->channel.hw_value = lastest_scan_result.channel;
			//conn_param->channel.center_freq = lastest_scan_result.channel;
	        privacy = conn_param->privacy = (lastest_scan_result.proto == WPA_PROTO_OPEN_SHARE)?TRUE:FALSE;
			bssid = conn_param->bssid = lastest_scan_result.bssid;
			if(lastest_scan_result.cipher & WPA_CIPHER_CCMP)
			{
				conn_param->crypto.n_ciphers_pairwise = 1;
				conn_param->crypto.ciphers_pairwise[0] = WLAN_CIPHER_SUITE_CCMP;
				conn_param->crypto.cipher_group = WLAN_CIPHER_SUITE_CCMP;
			}
			else if(lastest_scan_result.cipher & WPA_CIPHER_TKIP)
			{
				conn_param->crypto.n_ciphers_pairwise = 1;
				conn_param->crypto.ciphers_pairwise[0] = WLAN_CIPHER_SUITE_TKIP;
				conn_param->crypto.cipher_group = WLAN_CIPHER_SUITE_TKIP;
			}
			else if(lastest_scan_result.cipher & WPA_CIPHER_WEP40 || lastest_scan_result.cipher & WPA_CIPHER_WEP104)
			{
				
				conn_param->crypto.n_ciphers_pairwise = 1;
				if(conn_param->key_len == MIN_WEP_KEY_SIZE)
				{
					conn_param->crypto.ciphers_pairwise[0] = WLAN_CIPHER_SUITE_WEP40;
					conn_param->crypto.cipher_group = WLAN_CIPHER_SUITE_WEP40;
				}else{
					conn_param->crypto.ciphers_pairwise[0] = WLAN_CIPHER_SUITE_WEP104;
					conn_param->crypto.cipher_group = WLAN_CIPHER_SUITE_WEP104;
				}
				
				conn_param->auth_type = NL80211_AUTHTYPE_SHARED_KEY;
			}
			else
			{
				conn_param->crypto.n_ciphers_pairwise = 0;
				conn_param->crypto.ciphers_pairwise[0] = IW_AUTH_CIPHER_NONE;
				conn_param->crypto.cipher_group = IW_AUTH_CIPHER_NONE;
				conn_param->auth_type = NL80211_AUTHTYPE_OPEN_SYSTEM;
			}
			
			switch(lastest_scan_result.proto)
			{
				case WPA_PROTO_WPA:
					conn_param->crypto.wpa_versions = NL80211_WPA_VERSION_1;
					break;
				case WPA_PROTO_RSN:
					conn_param->crypto.wpa_versions = NL80211_WPA_VERSION_2;
					break;
				default:
					break;
			}
		}
	}
#endif

    if ((mode == MLAN_BSS_MODE_IBSS) && channel) {
        /* Get the secondary channel offset */
        req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_radio_cfg));
        if (req == NULL) {
            ret = -ENOMEM;
            goto done;
        }
        radio_cfg = (mlan_ds_radio_cfg *) req->pbuf;
        radio_cfg->sub_command = MLAN_OID_BAND_CFG;
        req->req_id = MLAN_IOCTL_RADIO_CFG;
        req->action = MLAN_ACT_GET;

        if (MLAN_STATUS_SUCCESS !=
            woal_request_ioctl(priv, req, MOAL_IOCTL_WAIT)) {
            ret = -EFAULT;
            goto done;
        }
        if (MLAN_STATUS_SUCCESS != woal_set_rf_channel(priv,
                                                       channel,
                                                       woal_channel_to_nl80211_channel_type
                                                       (radio_cfg->param.
                                                        band_cfg.
                                                        sec_chan_offset))) {
            ret = -EFAULT;
            goto done;
        }
    }

#if 1	//add by forrest 设置wpa密码
	if(conn_param->crypto.cipher_group == WLAN_CIPHER_SUITE_TKIP|| 
		conn_param->crypto.cipher_group == WLAN_CIPHER_SUITE_CCMP)
	{
		passphrase = (mlan_ds_passphrase*)mem_calloc(sizeof(mlan_ds_passphrase), 1);
		if(passphrase)
		{
			memcpy(passphrase->bssid, conn_param->bssid, 6);
			passphrase->bssid[5]=0;
			memcpy(passphrase->ssid.ssid, conn_param->ssid, 32);
			passphrase->ssid.ssid_len = conn_param->ssid_len;
			memcpy(passphrase->psk.passphrase.passphrase, conn_param->key, 32);
			passphrase->psk.passphrase.passphrase_len = conn_param->key_len;
			passphrase->psk_type = MLAN_PSK_PASSPHRASE;
			
			if (MLAN_STATUS_SUCCESS !=
	        	woal_set_passphrase(priv, MOAL_IOCTL_WAIT, passphrase)) {
	        	/* Disable keys and clear all previous security settings */
	        	ret = -EFAULT;
	        	goto done;
	    	}
			mem_free(passphrase);
		}
		if (MLAN_STATUS_SUCCESS !=
        	woal_set_ewpa_mode(priv, MOAL_IOCTL_WAIT, &ssid_bssid, TRUE)) {
        	ret = -EFAULT;
        	goto done;
    	}
		
	}else{
		
		if (MLAN_STATUS_SUCCESS !=
			woal_set_ewpa_mode(priv, MOAL_IOCTL_WAIT, &ssid_bssid, FALSE)) {
			ret = -EFAULT;
			goto done;
		}

	}
#endif
 

    if (MLAN_STATUS_SUCCESS !=
        woal_cfg80211_set_key(priv, 0, 0, NULL, 0, NULL, 0,
                              KEY_INDEX_CLEAR_ALL, NULL, 1)) {
        /* Disable keys and clear all previous security settings */
        ret = -EFAULT;
        goto done;
    }

    if (ie && ie_len) {         /* Set the IE */
        if (MLAN_STATUS_SUCCESS !=
            woal_cfg80211_assoc_ies_cfg(priv, ie, ie_len)) {
            ret = -EFAULT;
            goto done;
        }
    }

    if (conn_param && mode != MLAN_BSS_MODE_IBSS) {
        /* These parameters are only for managed mode */
        if (conn_param->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM)
            auth_type = MLAN_AUTH_MODE_OPEN;
        else if (conn_param->auth_type == NL80211_AUTHTYPE_SHARED_KEY)
            auth_type = MLAN_AUTH_MODE_SHARED;
        else if (conn_param->auth_type == NL80211_AUTHTYPE_NETWORK_EAP)
            auth_type = MLAN_AUTH_MODE_NETWORKEAP;
        else
            auth_type = MLAN_AUTH_MODE_AUTO;

        if (MLAN_STATUS_SUCCESS !=
            woal_set_auth_mode(priv, MOAL_IOCTL_WAIT, auth_type)) {
            ret = -EFAULT;
            goto done;
        }

        if (conn_param->crypto.n_ciphers_pairwise) {
            pairwise_enc_mode =
                woal_cfg80211_get_encryption_mode(conn_param->
                                                  crypto.ciphers_pairwise[0],
                                                  &wpa_enabled);
            ret = woal_cfg80211_set_auth(priv, pairwise_enc_mode, wpa_enabled);
            if (ret)
                goto done;
        }

        if (conn_param->crypto.cipher_group) {
            group_enc_mode =
                woal_cfg80211_get_encryption_mode(conn_param->
                                                  crypto.cipher_group,
                                                  &wpa_enabled);
            ret = woal_cfg80211_set_auth(priv, group_enc_mode, wpa_enabled);
            if (ret)
                goto done;
        }

        if (conn_param->key) {
            alg_is_wep =
                woal_cfg80211_is_alg_wep(pairwise_enc_mode) |
                woal_cfg80211_is_alg_wep(group_enc_mode);
            if (alg_is_wep) {
                PRINTM(MINFO, "Setting wep encryption with "
                       "key len %d\n", conn_param->key_len);
                /* Set the WEP key */
                if (MLAN_STATUS_SUCCESS !=
                    woal_cfg80211_set_wep_keys(priv, conn_param->key,
                                               conn_param->key_len,
                                               conn_param->key_idx)) {
                    ret = -EFAULT;
                    goto done;
                }
                /* Enable the WEP key by key index */
                if (MLAN_STATUS_SUCCESS !=
                    woal_cfg80211_set_wep_keys(priv, NULL, 0,
                                               conn_param->key_idx)) {
                    ret = -EFAULT;
                    goto done;
                }
            }
        }
    }

    if (mode == MLAN_BSS_MODE_IBSS) {
        mlan_ds_bss *bss = NULL;
        /* Change beacon interval */
        if ((beacon_interval < MLAN_MIN_BEACON_INTERVAL) ||
            (beacon_interval > MLAN_MAX_BEACON_INTERVAL)) {
            ret = -EINVAL;
            goto done;
        }
        if (req)
            kfree(req);
        req = NULL;

        req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_bss));
        if (req == NULL) {
            ret = -ENOMEM;
            goto done;
        }
        bss = (mlan_ds_bss *) req->pbuf;
        req->req_id = MLAN_IOCTL_BSS;
        req->action = MLAN_ACT_SET;
        bss->sub_command = MLAN_OID_IBSS_BCN_INTERVAL;
        bss->param.bcn_interval = beacon_interval;
        if (MLAN_STATUS_SUCCESS !=
            woal_request_ioctl(priv, req, MOAL_IOCTL_WAIT)) {
            ret = -EFAULT;
            goto done;
        }

        /* "privacy" is set only for ad-hoc mode */
        if (privacy) {
            /* 
             * Keep MLAN_ENCRYPTION_MODE_WEP40 for now so that
             * the firmware can find a matching network from the
             * scan. cfg80211 does not give us the encryption
             * mode at this stage so just setting it to wep here
             */
            if (MLAN_STATUS_SUCCESS !=
                woal_set_auth_mode(priv, MOAL_IOCTL_WAIT,
                                   MLAN_AUTH_MODE_SHARED)) {
                ret = -EFAULT;
                goto done;
            }

            wpa_enabled = 0;
            ret = woal_cfg80211_set_auth(priv,
                                         MLAN_ENCRYPTION_MODE_WEP104,
                                         wpa_enabled);
            if (ret)
                goto done;

			/* Set the WEP key */
                if (MLAN_STATUS_SUCCESS !=
                    woal_cfg80211_set_wep_keys(priv, (u8*)ibss_param->key,
                                               ibss_param->key_len,
                                               0)) {
                    ret = -EFAULT;
                    goto done;
                }
                /* Enable the WEP key by key index */
                if (MLAN_STATUS_SUCCESS !=
                    woal_cfg80211_set_wep_keys(priv, NULL, 0,
                                               0)) {
                    ret = -EFAULT;
                    goto done;
                }
        }
    }
    memcpy(&ssid_bssid.ssid, &req_ssid, sizeof(mlan_802_11_ssid));
    if (bssid)
        memcpy(&ssid_bssid.bssid, bssid, ETH_ALEN);
    if (MLAN_STATUS_SUCCESS != woal_find_essid(priv, &ssid_bssid)) {
        /* Do specific SSID scanning */
        if (MLAN_STATUS_SUCCESS !=
            woal_request_scan(priv, MOAL_IOCTL_WAIT, &req_ssid)) {
            ret = -EFAULT;
            goto done;
        }
    }

    /* Disconnect before try to associate */
	if (priv->media_connected == MTRUE) //add by forrest
    	woal_disconnect(priv, MOAL_IOCTL_WAIT, NULL);

    if (mode != MLAN_BSS_MODE_IBSS) {
        if (MLAN_STATUS_SUCCESS !=
            woal_find_best_network(priv, MOAL_IOCTL_WAIT, &ssid_bssid)) {
            ret = -EFAULT;
            goto done;
        }
        /* Inform the BSS information to kernel, otherwise kernel will give a
           panic after successful assoc */
        if (MLAN_STATUS_SUCCESS !=
            woal_inform_bss_from_scan_result(priv, &req_ssid)) {
            ret = -EFAULT;
            goto done;
        }
    } else if (MLAN_STATUS_SUCCESS !=
               woal_find_best_network(priv, MOAL_IOCTL_WAIT, &ssid_bssid))
        /* Adhoc start, Check the channel command */
        woal_11h_channel_check_ioctl(priv);

    PRINTM(MINFO, "Trying to associate to %s and bssid %pM\n",
           (char *) req_ssid.ssid, ssid_bssid.bssid);
#ifdef REASSOCIATION	
	priv->prev_ssid_bssid = ssid_bssid; //add by forrest
#endif	
    /* Zero SSID implies use BSSID to connect */
    if (bssid)
        memset(&ssid_bssid.ssid, 0, sizeof(mlan_802_11_ssid));
    else                        /* Connect to BSS by ESSID */
        memset(&ssid_bssid.bssid, 0, MLAN_MAC_ADDR_LENGTH);

    if (MLAN_STATUS_SUCCESS !=
        woal_bss_start(priv, MOAL_IOCTL_WAIT, &ssid_bssid)) {
        ret = -EFAULT;
        goto done;
    }

    

    
    /* Inform the IBSS information to kernel, otherwise kernel will give a
       panic after successful assoc */
    if (mode == MLAN_BSS_MODE_IBSS) {
        if (MLAN_STATUS_SUCCESS !=
            woal_cfg80211_inform_ibss_bss(priv, channel, beacon_interval)) {
            ret = -EFAULT;
            goto done;
        }
    }

  done:
    if (ret) {
        /* clear IE */
        ie_len = 0;
        woal_set_get_gen_ie(priv, MLAN_ACT_SET, NULL, &ie_len);
    }
    if (req)
        kfree(req);
    LEAVE();
    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0) || defined(COMPAT_WIRELESS)
/** 
 *  @brief Set/Get DTIM period
 *
 *  @param priv                 A pointer to moal_private structure
 *  @param action               Action set or get
 *  @param wait_option          Wait option
 *  @param value                DTIM period
 *
 *  @return                     MLAN_STATUS_SUCCESS/MLAN_STATUS_PENDING -- success, otherwise fail
 */
static mlan_status
woal_set_get_dtim_period(moal_private * priv,
                         t_u32 action, t_u8 wait_option, t_u8 * value)
{
    mlan_ioctl_req *req = NULL;
    mlan_ds_snmp_mib *mib = NULL;
    mlan_status ret = MLAN_STATUS_SUCCESS;

    ENTER();

    /* Allocate an IOCTL request buffer */
    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_snmp_mib));
    if (req == NULL) {
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }

    /* Fill request buffer */
    mib = (mlan_ds_snmp_mib *) req->pbuf;
    mib->sub_command = MLAN_OID_SNMP_MIB_DTIM_PERIOD;
    req->req_id = MLAN_IOCTL_SNMP_MIB;
    req->action = action;

    if (action == MLAN_ACT_SET) {
        mib->param.dtim_period = *value;
    }

    /* Send IOCTL request to MLAN */
    ret = woal_request_ioctl(priv, req, wait_option);
    if (ret == MLAN_STATUS_SUCCESS && action == MLAN_ACT_GET) {
        *value = (t_u8) mib->param.dtim_period;
    }

  done:
    if (req && (ret != MLAN_STATUS_PENDING))
        kfree(req);
    LEAVE();
    return ret;
}
#endif

/**
 * @brief Request the driver to dump the station information
 *
 * @param priv            A pointer to moal_private structure
 * @param sinfo           A pointer to station_info structure
 *
 * @return                0 -- success, otherwise fail
 */
static mlan_status
woal_cfg80211_dump_station_info(moal_private * priv, struct station_info *sinfo)
{
    mlan_status ret = MLAN_STATUS_SUCCESS;
    mlan_ds_get_signal signal;
    mlan_ioctl_req *req = NULL;
    mlan_ds_rate *rate = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0) || defined(COMPAT_WIRELESS)
    mlan_bss_info bss_info;
    t_u8 dtim_period = 0;
#endif

    ENTER();
    sinfo->filled = STATION_INFO_RX_BYTES | STATION_INFO_TX_BYTES |
        STATION_INFO_RX_PACKETS | STATION_INFO_TX_PACKETS |
        STATION_INFO_SIGNAL | STATION_INFO_TX_BITRATE;

    /* Get signal information from the firmware */
    memset(&signal, 0, sizeof(mlan_ds_get_signal));
    if (MLAN_STATUS_SUCCESS !=
        woal_get_signal_info(priv, MOAL_IOCTL_WAIT, &signal)) {
        PRINTM(MERROR, "Error getting signal information\n");
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }

    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_rate));
    if (req == NULL) {
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }
    rate = (mlan_ds_rate *) req->pbuf;
    rate->sub_command = MLAN_OID_GET_DATA_RATE;
    req->req_id = MLAN_IOCTL_RATE;
    req->action = MLAN_ACT_GET;
    if (MLAN_STATUS_SUCCESS != woal_request_ioctl(priv, req, MOAL_IOCTL_WAIT)) {
        ret = MLAN_STATUS_FAILURE;
        goto done;
    }
    sinfo->txrate.flags = RATE_INFO_FLAGS_MCS;
    if (rate->param.data_rate.tx_ht_bw == MLAN_HT_BW40) {
        sinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
    }
    if (rate->param.data_rate.tx_ht_gi == MLAN_HT_SGI) {
        sinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
    }
    sinfo->txrate.mcs = rate->param.data_rate.tx_data_rate;
    sinfo->rx_bytes = priv->stats.rx_bytes;
    sinfo->tx_bytes = priv->stats.tx_bytes;
    sinfo->rx_packets = priv->stats.rx_packets;
    sinfo->tx_packets = priv->stats.tx_packets;
    sinfo->signal = signal.bcn_rssi_avg;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0) || defined(COMPAT_WIRELESS)
    /* Update BSS information */
    sinfo->filled |= STATION_INFO_BSS_PARAM;
    sinfo->bss_param.flags = 0;
    ret = woal_get_bss_info(priv, MOAL_IOCTL_WAIT, &bss_info);
    if (ret)
        goto done;
    if (bss_info.capability_info & WLAN_CAPABILITY_SHORT_PREAMBLE)
        sinfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_PREAMBLE;
    if (bss_info.capability_info & WLAN_CAPABILITY_SHORT_SLOT_TIME)
        sinfo->bss_param.flags |= BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
    sinfo->bss_param.beacon_interval = bss_info.beacon_interval;
    /* Get DTIM period */
    ret = woal_set_get_dtim_period(priv, MLAN_ACT_GET,
                                   MOAL_IOCTL_WAIT, &dtim_period);
    if (ret) {
        PRINTM(MERROR, "Get DTIM period failed\n");
        goto done;
    }
    sinfo->bss_param.dtim_period = dtim_period;
#endif

  done:
    if (req)
        kfree(req);

    LEAVE();
    return ret;
}

/********************************************************
				Global Functions
********************************************************/
#if 0
/**
 * @brief Request the driver to change regulatory domain
 *
 * @param wiphy           A pointer to wiphy structure
 * @param request         A pointer to regulatory_request structure
 *
 * @return                0
 */
static int
woal_cfg80211_reg_notifier(struct wiphy *wiphy,
                           struct regulatory_request *request)
{
    moal_private *priv = (moal_private *) woal_get_wiphy_priv(wiphy);
    int ret = 0;

    ENTER();

    if (!priv) {
        PRINTM(MFATAL, "Unable to get priv in %s()\n", __FUNCTION__);
        LEAVE();
        return -EINVAL;
    }

    PRINTM(MINFO, "cfg80211 regulatory domain callback "
           "%c%c\n", request->alpha2[0], request->alpha2[1]);

    if (MLAN_STATUS_SUCCESS != woal_set_region_code(priv, request->alpha2))
        PRINTM(MERROR, "Set country code failed!\n");
    memcpy(priv->country_code, request->alpha2, COUNTRY_CODE_LEN);

    switch (request->initiator) {
    case NL80211_REGDOM_SET_BY_DRIVER:
        PRINTM(MINFO, "Regulatory domain BY_DRIVER\n");
        break;
    case NL80211_REGDOM_SET_BY_CORE:
        PRINTM(MINFO, "Regulatory domain BY_CORE\n");
        break;
    case NL80211_REGDOM_SET_BY_USER:
        PRINTM(MINFO, "Regulatory domain BY_USER\n");
        break;
        /* TODO: apply driver specific changes in channel flags based on the
           request initiator if necessory. * */
    case NL80211_REGDOM_SET_BY_COUNTRY_IE:
        PRINTM(MINFO, "Regulatory domain BY_COUNTRY_IE\n");
        break;
    }

    if (MLAN_STATUS_SUCCESS != woal_send_domain_info_cmd_fw(priv))
        ret = -EFAULT;

    LEAVE();
    return ret;
}
#endif
/**
 * @brief Request the driver to do a scan. Always returning
 * zero meaning that the scan request is given to driver,
 * and will be valid until passed to cfg80211_scan_done().
 * To inform scan results, call cfg80211_inform_bss().
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 * @param request         A pointer to cfg80211_scan_request structure
 *
 * @return                0 -- success, otherwise fail
 */
static int
woal_cfg80211_scan(struct wiphy *wiphy, struct net_device *dev,
                   struct cfg80211_scan_request *request)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);
    wlan_user_scan_cfg scan_req;
    mlan_bss_info bss_info;
    struct ieee80211_channel *chan;
    int ret = 0, i;

    ENTER();

    PRINTM(MINFO, "Received scan request on %s\n", dev->name);
#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
    if (GET_BSS_ROLE(priv) == MLAN_BSS_ROLE_UAP) {
        LEAVE();
        cfg80211_scan_done(request, MTRUE);
        return 0;
    }
#endif
#endif
    if (priv->phandle->scan_pending_on_block == MTRUE) {
        PRINTM(MINFO, "scan already in processing...\n");
        LEAVE();
        return -EBUSY;
    }
#if defined(STA_CFG80211) || defined(UAP_CFG80211)
#ifdef WIFI_DIRECT_SUPPORT
    if (priv->phandle->remain_on_channel || priv->phandle->is_go_timer_set) {
        PRINTM(MINFO, "block scan ... remain_on_channel=%d go_timer=%d\n",
               priv->phandle->remain_on_channel,
               priv->phandle->is_go_timer_set);
        LEAVE();
        return -EBUSY;
    }
#endif
#endif
    memset(&bss_info, 0, sizeof(bss_info));
    if (MLAN_STATUS_SUCCESS ==
        woal_get_bss_info(priv, MOAL_IOCTL_WAIT, &bss_info)) {
        if (bss_info.scan_block) {
            PRINTM(MINFO, "block scan in mlan module...\n");
            LEAVE();
            return -EBUSY;
        }
    }
    if (priv->scan_request && priv->scan_request != request) {
        LEAVE();
        return -EBUSY;
    }
    priv->scan_request = request;

    memset(&scan_req, 0x00, sizeof(scan_req));
    for (i = 0; i < priv->scan_request->n_ssids; i++) {
        memcpy(scan_req.ssid_list[i].ssid,
               priv->scan_request->ssids[i].ssid,
               priv->scan_request->ssids[i].ssid_len);
        if (priv->scan_request->ssids[i].ssid_len)
            scan_req.ssid_list[i].max_len = 0;
        else
            scan_req.ssid_list[i].max_len = 0xff;
        PRINTM(MIOCTL, "scan: ssid=%s\n", scan_req.ssid_list[i].ssid);
    }
#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
    if (priv->bss_type == MLAN_BSS_TYPE_WIFIDIRECT &&
        priv->scan_request->n_ssids) {
        if (!memcmp(scan_req.ssid_list[i - 1].ssid, "DIRECT-", 7)) {
            /* Enable wildcard ssid scan */
            memcpy(scan_req.ssid_list[i].ssid, "DIRECT-*", 8);
            scan_req.ssid_list[i].max_len = 0xff;
        }
    }
#endif
#endif
    for (i = 0; i < priv->scan_request->n_channels; i++) {
        chan = priv->scan_request->channels[i];
        scan_req.chan_list[i].chan_number = chan->hw_value;
        scan_req.chan_list[i].radio_type = chan->band;
        if (chan->flags & IEEE80211_CHAN_DISABLED)
            scan_req.chan_list[i].scan_type = MLAN_SCAN_TYPE_PASSIVE;
        else
            scan_req.chan_list[i].scan_type = MLAN_SCAN_TYPE_ACTIVE;
        scan_req.chan_list[i].scan_time = 0;
    }
    if (priv->scan_request->ie && priv->scan_request->ie_len) {
        if (MLAN_STATUS_SUCCESS !=
            woal_cfg80211_mgmt_frame_ie(priv, NULL, 0,
                                        NULL, 0, NULL, 0,
                                        (t_u8 *) priv->scan_request->ie,
                                        priv->scan_request->ie_len,
                                        MGMT_MASK_PROBE_REQ)) {
            PRINTM(MERROR, "Fail to set mgmt frame IE\n");
            ret = -EFAULT;
            goto done;
        }
    } else {
                /** Clear SCAN IE in Firmware */
        woal_cfg80211_mgmt_frame_ie(priv, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                                    MGMT_MASK_PROBE_REQ);
    }
    if (MLAN_STATUS_SUCCESS != woal_do_scan(priv, &scan_req)) {
        PRINTM(MERROR, "woal_do_scan fails!\n");
        ret = -EAGAIN;
        goto done;
    }
  done:
    if (ret)
        priv->scan_request = NULL;
    LEAVE();
    return ret;
}

extern void cfg80211_connect_result(struct net_device *dev, const u8 *bssid,
			     const u8 *req_ie, size_t req_ie_len,
			     const u8 *resp_ie, size_t resp_ie_len,
			     u16 status, gfp_t gfp);
/**
 * @brief Request the driver to connect to the ESS with
 * the specified parameters from kernel
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 * @param sme             A pointer to cfg80211_connect_params structure
 *
 * @return                0 -- success, otherwise fail
 */
static int woal_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
                      struct cfg80211_connect_params *sme)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);
    int ret = 0;

    ENTER();

    PRINTM(MINFO, "Received association request on %s\n", dev->name);

    if (priv->wdev->iftype != NL80211_IFTYPE_STATION
#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37) || defined(COMPAT_WIRELESS)
        && priv->wdev->iftype != NL80211_IFTYPE_P2P_CLIENT
#endif /* KERNEL_VERSION */
#endif /* WIFI_DIRECT_SUPPORT && V14_FEATURE */
        ) {
        PRINTM(MERROR, "Received infra assoc request "
               "when station not in infra mode\n");
        LEAVE();
        return -EINVAL;
    }
        /** cancel pending scan */
    woal_cancel_scan(priv, MOAL_IOCTL_WAIT);
#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
    if (priv->bss_type == MLAN_BSS_TYPE_WIFIDIRECT
        && (priv->wdev->iftype == NL80211_IFTYPE_STATION
            || priv->wdev->iftype == NL80211_IFTYPE_P2P_CLIENT)) {
        /* if bsstype == wifi direct, and iftype == station or p2p client, that 
           means wpa_supplicant wants to enable wifi direct functionality, so
           we should init p2p client. Note that due to kernel iftype check,
           ICS wpa_supplicant could not updaet iftype to init p2p client, so
           we have to done it here. */
        if (MLAN_STATUS_SUCCESS != woal_cfg80211_init_p2p_client(priv)) {
            PRINTM(MERROR, "Init p2p client for wpa_supplicant failed.\n");
            ret = -EFAULT;

            LEAVE();
            return ret;
        }
    }
#endif
#endif

    if (priv->scan_type == MLAN_SCAN_TYPE_PASSIVE)
        woal_set_scan_type(priv, MLAN_SCAN_TYPE_ACTIVE);
    ret = woal_cfg80211_assoc(priv, (void *) sme);

    if (priv->scan_type == MLAN_SCAN_TYPE_PASSIVE)
        woal_set_scan_type(priv, MLAN_SCAN_TYPE_PASSIVE);

    if (!ret) {
        cfg80211_connect_result(priv->netdev, priv->cfg_bssid, NULL, 0,
                                NULL, 0, WLAN_STATUS_SUCCESS, GFP_KERNEL);
        PRINTM(MINFO, "Associated to bssid %pM successfully\n",
               priv->cfg_bssid);
    } else {
        PRINTM(MINFO, "Association to bssid %pM failed\n", priv->cfg_bssid);
        cfg80211_connect_result(priv->netdev, priv->cfg_bssid, NULL, 0,
                                NULL, 0, WLAN_STATUS_UNSPECIFIED_FAILURE,
                                GFP_KERNEL);
        memset(priv->cfg_bssid, 0, ETH_ALEN);
    }

    LEAVE();
    return ret;
}

/**
 * @brief Request the driver to disconnect
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 * @param reason_code     Reason code
 *
 * @return                0 -- success, otherwise fail
 */
static int
woal_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev,
                         t_u16 reason_code)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);

    ENTER();
    PRINTM(MINFO, "Received disassociation request on %s\n", dev->name);

    if (priv->cfg_disconnect) {
        PRINTM(MERROR, "Disassociation already in progress\n");
        LEAVE();
        return -EBUSY;
    }

    if (priv->media_connected == MFALSE) {
        LEAVE();
        return -EINVAL;
    }

    priv->cfg_disconnect = TRUE;

    if (woal_disconnect(priv, MOAL_IOCTL_WAIT, priv->cfg_bssid) !=
        MLAN_STATUS_SUCCESS) {
        LEAVE();
        return -EFAULT;
    }

    PRINTM(MINFO, "Successfully disconnected from %pM: Reason code %d\n",
           priv->cfg_bssid, reason_code);

    memset(priv->cfg_bssid, 0, ETH_ALEN);

    LEAVE();
    return 0;
}

/**
 * @brief Request the driver to get the station information
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 * @param mac             MAC address of the station
 * @param sinfo           A pointer to station_info structure
 *
 * @return                0 -- success, otherwise fail
 */
static int
woal_cfg80211_get_station(struct wiphy *wiphy,
                          struct net_device *dev,
                          const u8 * mac, struct station_info *sinfo)
{
    mlan_status ret = MLAN_STATUS_SUCCESS;
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);

    ENTER();

#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
    if (GET_BSS_ROLE(priv) == MLAN_BSS_ROLE_UAP) {
        LEAVE();
        return woal_uap_cfg80211_get_station(wiphy, dev, (u8*)mac, sinfo);
    }
#endif
#endif
    if (priv->media_connected == MFALSE) {
        PRINTM(MINFO, "cfg80211: Media not connected!\n");
        LEAVE();
        return -ENOENT;
    }
    if (memcmp(mac, priv->cfg_bssid, ETH_ALEN)) {
        PRINTM(MINFO, "cfg80211: Request not for this station!\n");
        LEAVE();
        return -ENOENT;
    }

    if (MLAN_STATUS_SUCCESS != woal_cfg80211_dump_station_info(priv, sinfo)) {
        PRINTM(MERROR, "cfg80211: Failed to get station info\n");
        ret = MLAN_STATUS_FAILURE;
    }

    LEAVE();
    return ret;
}

/**
 * @brief Request the driver to dump the station information
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 * @param idx             Station index
 * @param mac             MAC address of the station
 * @param sinfo           A pointer to station_info structure
 *
 * @return                0 -- success, otherwise fail
 */
static int
woal_cfg80211_dump_station(struct wiphy *wiphy,
                           struct net_device *dev, int idx,
                           t_u8 * mac, struct station_info *sinfo)
{
    mlan_status ret = MLAN_STATUS_SUCCESS;
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);

    ENTER();

    if (!priv->media_connected || idx != 0) {
        PRINTM(MINFO, "cfg80211: Media not connected or"
               " not for this station!\n");
        LEAVE();
        return -ENOENT;
    }

    memcpy(mac, priv->cfg_bssid, ETH_ALEN);

    if (MLAN_STATUS_SUCCESS != woal_cfg80211_dump_station_info(priv, sinfo)) {
        PRINTM(MERROR, "cfg80211: Failed to get station info\n");
        ret = MLAN_STATUS_FAILURE;
    }

    LEAVE();
    return ret;
}

extern void cfg80211_ibss_joined(struct net_device *dev, const u8 *bssid, gfp_t gfp);
/**
 * @brief Request the driver to Join the specified
 * IBSS (or create if necessary)
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 * @param params          A pointer to cfg80211_ibss_params structure
 *
 * @return                0 -- success, otherwise fail
 */
static int
woal_cfg80211_join_ibss(struct wiphy *wiphy, struct net_device *dev,
                        struct cfg80211_ibss_params *params)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);
    int ret = 0;

    ENTER();

    if (priv->wdev->iftype != NL80211_IFTYPE_ADHOC) {
        PRINTM(MERROR, "Request IBSS join received "
               "when station not in ibss mode\n");
        LEAVE();
        return -EINVAL;
    }

    ret = woal_cfg80211_assoc(priv, (void *) params);

    if (!ret) {
        cfg80211_ibss_joined(priv->netdev, priv->cfg_bssid, GFP_KERNEL);
        PRINTM(MINFO, "Joined/created adhoc network with bssid"
               " %pM successfully\n", priv->cfg_bssid);
    } else {
        PRINTM(MINFO, "Failed creating/joining adhoc network\n");
        memset(priv->cfg_bssid, 0, ETH_ALEN);
    }

    LEAVE();
    return ret;
}

/**
 * @brief Request the driver to leave the IBSS
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 *
 * @return                0 -- success, otherwise fail
 */
static int
woal_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);

    ENTER();

    if (priv->cfg_disconnect) {
        PRINTM(MERROR, "IBSS leave already in progress\n");
        LEAVE();
        return -EBUSY;
    }

    if (priv->media_connected == MFALSE) {
        LEAVE();
        return -EINVAL;
    }

    priv->cfg_disconnect = TRUE;

    PRINTM(MINFO, "Leaving from IBSS %pM\n", priv->cfg_bssid);
    if (woal_disconnect(priv, MOAL_IOCTL_WAIT, priv->cfg_bssid) !=
        MLAN_STATUS_SUCCESS) {
        LEAVE();
        return -EFAULT;
    }

    memset(priv->cfg_bssid, 0, ETH_ALEN);

    LEAVE();
    return 0;
}

/**
 * @brief Request the driver to change the IEEE power save
 * mdoe
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 * @param enabled         Enable or disable
 * @param timeout         Timeout value
 *
 * @return                0 -- success, otherwise fail
 */
static int
woal_cfg80211_set_power_mgmt(struct wiphy *wiphy,
                             struct net_device *dev, bool enabled, int timeout)
{
    int ret = 0, disabled;
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);

    ENTER();

    if (enabled)
        disabled = 0;
    else
        disabled = 1;

    if (MLAN_STATUS_SUCCESS !=
        woal_set_get_power_mgmt(priv, MLAN_ACT_SET, &disabled, timeout)) {
        ret = -EOPNOTSUPP;
    }

    LEAVE();
    return ret;
}

/**
 * @brief Request the driver to change the transmit power
 *
 * @param wiphy           A pointer to wiphy structure
 * @param type            TX power adjustment type
 * @param dbm             TX power in dbm
 *
 * @return                0 -- success, otherwise fail
 */
int
woal_cfg80211_set_tx_power(struct wiphy *wiphy,
							struct net_device *dev,
                           enum nl80211_tx_power_setting type,
                           int dbm)
{
    int ret = 0;
    moal_private *priv = (moal_private *) woal_get_wiphy_priv(wiphy);
    mlan_power_cfg_t power_cfg;

    ENTER();

    if (!priv) {
        PRINTM(MFATAL, "Unable to get priv in %s()\n", __FUNCTION__);
        LEAVE();
        return -EFAULT;
    }

    if (type) {
        power_cfg.is_power_auto = 0;
        power_cfg.power_level = dbm;
    } else
        power_cfg.is_power_auto = 1;

    if (MLAN_STATUS_SUCCESS !=
        woal_set_get_tx_power(priv, MLAN_ACT_SET, &power_cfg))
        ret = -EFAULT;

    LEAVE();
    return ret;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35) || defined(COMPAT_WIRELESS)
/**
 * CFG802.11 operation handler for connection quality monitoring.
 *
 * @param wiphy           A pointer to wiphy structure
 * @param dev             A pointer to net_device structure
 * @param rssi_thold	  rssi threshold
 * @param rssi_hyst		  rssi hysteresis
 */
static int
woal_cfg80211_set_cqm_rssi_config(struct wiphy *wiphy,
                                  struct net_device *dev,
                                  s32 rssi_thold, u32 rssi_hyst)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);
    ENTER();
    priv->cqm_rssi_thold = rssi_thold;
    priv->cqm_rssi_hyst = rssi_hyst;

    PRINTM(MIOCTL, "rssi_thold=%d rssi_hyst=%d\n",
           (int) rssi_thold, (int) rssi_hyst);
    woal_set_rssi_threshold(priv, 0);
    LEAVE();
    return 0;
}
#endif

#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
/**
 * @brief remain on channel config
 *
 * @param priv              A pointer to moal_private structure
 * @param wait_option       Wait option
 * @param cancel			cancel remain on channel flag            
 * @param status            A pointer to status, success, in process or reject
 * @param chan              A pointer to ieee80211_channel structure
 * @param channel_type      channel_type,
 * @param duration          Duration wait to receive frame
 *
 * @return                  0 -- success, otherwise fail
 */
int
woal_cfg80211_remain_on_channel_cfg(moal_private * priv,
                                    t_u8 wait_option, t_u8 remove,
                                    t_u8 * status,
                                    struct ieee80211_channel *chan,
                                    enum nl80211_channel_type channel_type,
                                    t_u32 duration)
{
    mlan_ds_remain_chan chan_cfg;
    int ret = 0;

    ENTER();
    memset(&chan_cfg, 0, sizeof(mlan_ds_remain_chan));
    if (remove) {
        chan_cfg.remove = MTRUE;
    } else {
        if (chan->band == IEEE80211_BAND_2GHZ)
            chan_cfg.bandcfg = 0;
        else if (chan->band == IEEE80211_BAND_5GHZ)
            chan_cfg.bandcfg = 1;
        switch (channel_type) {
        case NL80211_CHAN_HT40MINUS:
            chan_cfg.bandcfg |= SEC_CHANNEL_BELOW;
            break;
        case NL80211_CHAN_HT40PLUS:
            chan_cfg.bandcfg |= SEC_CHANNEL_ABOVE;
            break;

        case NL80211_CHAN_NO_HT:
        case NL80211_CHAN_HT20:
        default:
            break;
        }
        chan_cfg.channel = ieee80211_frequency_to_channel(chan->center_freq);
        chan_cfg.remain_period = duration;
    }
    if (MLAN_STATUS_SUCCESS ==
        woal_set_remain_channel_ioctl(priv, wait_option, &chan_cfg))
        *status = chan_cfg.status;
    else
        ret = -EFAULT;
    LEAVE();
    return ret;
}

/**
 * @brief tx mgmt frame
 *
 * @param wiphy                 A pointer to wiphy structure
 * @param dev                   A pointer to net_device structure
 * @param cookie                A pointer to frame cookie
 *
 * @return                0 -- success, otherwise fail
 */
static int
woal_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
                                  struct net_device *dev, u64 cookie)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);
    int ret = 0;
    t_u8 status = 1;

    ENTER();

    if (priv->phandle->remain_on_channel) {
        if (woal_cfg80211_remain_on_channel_cfg(priv,
                                                MOAL_IOCTL_WAIT, MTRUE, &status,
                                                NULL, 0, 0)) {
            PRINTM(MERROR, "Fail to cancel remain on channel\n");
            ret = -EFAULT;
            goto done;
        }
        if (priv->phandle->cookie) {
            cfg80211_remain_on_channel_expired(priv->netdev,
                                               priv->phandle->cookie,
                                               &priv->phandle->chan,
                                               priv->phandle->channel_type,
                                               GFP_ATOMIC);
            priv->phandle->cookie = 0;
        }
        priv->phandle->remain_on_channel = MFALSE;
    }

  done:
    LEAVE();
    return ret;
}

/**
 * @brief Make chip remain on channel
 *
 * @param wiphy                 A pointer to wiphy structure
 * @param dev                   A pointer to net_device structure
 * @param chan                  A pointer to ieee80211_channel structure
 * @param channel_type          Channel type
 * @param duration              Duration for timer
 * @param cookie                A pointer to timer cookie
 *
 * @return                  0 -- success, otherwise fail
 */
static int
woal_cfg80211_remain_on_channel(struct wiphy *wiphy,
                                struct net_device *dev,
                                struct ieee80211_channel *chan,
                                enum nl80211_channel_type channel_type,
                                unsigned int duration, u64 * cookie)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);
    int ret = 0;
    t_u8 status = 1;

    ENTER();

    if (!chan || !cookie) {
        PRINTM(MERROR, "Invalid parameter for remain on channel\n");
        ret = -EFAULT;
        goto done;
    }
        /** cancel previous remain on channel */
    if (priv->phandle->remain_on_channel) {
        if (woal_cfg80211_remain_on_channel_cfg(priv,
                                                MOAL_IOCTL_WAIT, MTRUE, &status,
                                                NULL, 0, 0)) {
            PRINTM(MERROR, "Fail to cancel remain on channel\n");
            ret = -EFAULT;
            goto done;
        }
        priv->phandle->cookie = 0;
        priv->phandle->remain_on_channel = MFALSE;
    }
        /** cancel pending scan */
    woal_cancel_scan(priv, MOAL_IOCTL_WAIT);
    if (MLAN_STATUS_SUCCESS !=
        woal_cfg80211_remain_on_channel_cfg(priv, MOAL_IOCTL_WAIT,
                                            MFALSE, &status, chan, channel_type,
                                            (t_u32) duration)) {
        ret = -EFAULT;
        goto done;
    }

    if (status == 0) {
        /* remain on channel operation success */
        /* we need update the value cookie */
        *cookie = (u64) random32() | 1;
        priv->phandle->remain_on_channel = MTRUE;
        priv->phandle->cookie = *cookie;
        priv->phandle->channel_type = channel_type;
        memcpy(&priv->phandle->chan, chan, sizeof(struct ieee80211_channel));
        cfg80211_ready_on_channel(dev, *cookie, chan,
                                  channel_type, duration, GFP_KERNEL);
        PRINTM(MIOCTL, "Set remain on Channel: channel=%d cookie = %#llx\n",
               ieee80211_frequency_to_channel(chan->center_freq),
               priv->phandle->cookie);
    }
  done:
    LEAVE();
    return ret;
}

/**
 * @brief Cancel remain on channel
 *
 * @param wiphy                 A pointer to wiphy structure
 * @param dev                   A pointer to net_device structure
 * @param cookie                A pointer to timer cookie
 *
 * @return                  0 -- success, otherwise fail
 */
static int
woal_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
                                       struct net_device *dev, u64 cookie)
{
    moal_private *priv = (moal_private *) woal_get_netdev_priv(dev);
    int ret = 0;
    t_u8 status = 1;

    ENTER();
    PRINTM(MIOCTL, "Cancel remain on Channel: cookie = %#llx\n", cookie);
    if (woal_cfg80211_remain_on_channel_cfg(priv,
                                            MOAL_IOCTL_WAIT, MTRUE, &status,
                                            NULL, 0, 0)) {
        PRINTM(MERROR, "Fail to cancel remain on channel\n");
        ret = -EFAULT;
        goto done;
    }

    priv->phandle->remain_on_channel = MFALSE;
    if (priv->phandle->cookie)
        priv->phandle->cookie = 0;
  done:
    LEAVE();
    return ret;
}
#endif /* KERNEL_VERSION */
#endif /* WIFI_DIRECT_SUPPORT && V14_FEATURE */

/**
 * @brief Initialize the wiphy
 *
 * @param priv            A pointer to moal_private structure
 * @param wait_option     Wait option
 *
 * @return                MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
mlan_status
woal_cfg80211_sta_init_wiphy(moal_private * priv, t_u8 wait_option)
{
    int retry_count, rts_thr, frag_thr, disabled;
    struct wiphy *wiphy = NULL;
    mlan_ioctl_req *req = NULL;
    mlan_ds_11n_cfg *cfg_11n = NULL;
    t_u32 hw_dev_cap;

    ENTER();

    if (priv->wdev)
        wiphy = priv->wdev->wiphy;
    else {
        //PRINTM(MERROR, "Invalid parameter when init wiphy.\n");
        goto done;
    }

    /* Get 11n tx parameters from MLAN */
    req = woal_alloc_mlan_ioctl_req(sizeof(mlan_ds_11n_cfg));
    if (req == NULL) {
        LEAVE();
        return MLAN_STATUS_FAILURE;
    }
    cfg_11n = (mlan_ds_11n_cfg *) req->pbuf;
    cfg_11n->sub_command = MLAN_OID_11N_HTCAP_CFG;
    req->req_id = MLAN_IOCTL_11N_CFG;
    req->action = MLAN_ACT_GET;
    cfg_11n->param.htcap_cfg.hw_cap_req = MTRUE;

    if (MLAN_STATUS_SUCCESS != woal_request_ioctl(priv, req, wait_option)) {
        kfree(req);
        LEAVE();
        return MLAN_STATUS_FAILURE;
    }
    hw_dev_cap = cfg_11n->param.htcap_cfg.htcap;

    /* Get supported MCS sets */
    memset(req->pbuf, 0, sizeof(mlan_ds_11n_cfg));
    cfg_11n->sub_command = MLAN_OID_11N_CFG_SUPPORTED_MCS_SET;
    req->req_id = MLAN_IOCTL_11N_CFG;
    req->action = MLAN_ACT_GET;

    if (MLAN_STATUS_SUCCESS != woal_request_ioctl(priv, req, wait_option)) {
        kfree(req);
        LEAVE();
        return MLAN_STATUS_FAILURE;
    }

    /* Initialize parameters for 2GHz and 5GHz bands */
    woal_cfg80211_setup_ht_cap(&wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap,
                               hw_dev_cap, cfg_11n->param.supported_mcs_set);
    /* For 2.4G band only card, this shouldn't be set */
    if (wiphy->bands[IEEE80211_BAND_5GHZ])
        woal_cfg80211_setup_ht_cap(&wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap,
                                   hw_dev_cap,
                                   cfg_11n->param.supported_mcs_set);
    if (req)
        kfree(req);

    /* Set retry limit count to wiphy */
    if (MLAN_STATUS_SUCCESS !=
        woal_set_get_retry(priv, MLAN_ACT_GET, wait_option, &retry_count)) {
        LEAVE();
        return MLAN_STATUS_FAILURE;
    }
    //wiphy->retry_long = (t_u8) retry_count;
    //wiphy->retry_short = (t_u8) retry_count;
    //wiphy->max_scan_ie_len = MAX_IE_SIZE;

#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
    /* we should add mgmt_stypes for both STA & Wifi Direct, while only
       initialize duration_timer for Wifi Direct */
    wiphy->mgmt_stypes = (struct ieee80211_txrx_stypes*)ieee80211_mgmt_stypes;
    wiphy->max_remain_on_channel_duration = MAX_REMAIN_ON_CHANNEL_DURATION;
#endif /* KERNEL_VERSION */
#endif /* WIFI_DIRECT_SUPPORT && V14_FEATURE */

    /* Set RTS threshold to wiphy */
    if (MLAN_STATUS_SUCCESS !=
        woal_set_get_rts(priv, MLAN_ACT_GET, wait_option, &rts_thr)) {
        LEAVE();
        return MLAN_STATUS_FAILURE;
    }
    if (rts_thr < MLAN_RTS_MIN_VALUE || rts_thr > MLAN_RTS_MAX_VALUE)
        rts_thr = MLAN_FRAG_RTS_DISABLED;
    //wiphy->rts_threshold = (t_u32) rts_thr;
    /* Set fragment threshold to wiphy */
    if (MLAN_STATUS_SUCCESS !=
        woal_set_get_frag(priv, MLAN_ACT_GET, wait_option, &frag_thr)) {
        LEAVE();
        return MLAN_STATUS_FAILURE;
    }
    if (frag_thr < MLAN_RTS_MIN_VALUE || frag_thr > MLAN_RTS_MAX_VALUE)
        frag_thr = MLAN_FRAG_RTS_DISABLED;
    //wiphy->frag_threshold = (t_u32) frag_thr;
    /* Get IEEE power save mode */
    if (MLAN_STATUS_SUCCESS != woal_set_get_power_mgmt(priv,
                                                       MLAN_ACT_GET, &disabled,
                                                       0)) {
        LEAVE();
        return MLAN_STATUS_FAILURE;
    }
    /* Save the IEEE power save mode to wiphy, because after warmreset wiphy
       power save should be updated instead of using the last saved
       configuration */
    if (disabled)
        priv->wdev->ps = FALSE;
    else
        priv->wdev->ps = TRUE;

  done:
    LEAVE();
    return MLAN_STATUS_SUCCESS;
}

/**
 * @brief Register the device with cfg80211
 *
 * @param dev       A pointer to net_device structure
 * @param bss_type  BSS type
 *
 * @return          MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */

mlan_status woal_register_sta_cfg80211(struct net_device * dev, t_u8 bss_type)
{
    mlan_status ret = MLAN_STATUS_SUCCESS;
    moal_private *priv = (moal_private *) netdev_priv(dev);
    //void *wdev_priv = NULL;
    struct wireless_dev *wdev = NULL;
    mlan_fw_info fw_info;
	
    ENTER();

    /* Allocate wireless device */
    wdev = kzalloc(sizeof(struct wireless_dev), GFP_KERNEL);
    if (!wdev) {
        PRINTM(MERROR, "Could not allocate wireless device\n");
        ret = MLAN_STATUS_FAILURE;
        goto err_wdev;
    }

    if (bss_type == MLAN_BSS_TYPE_STA)
	{
        /* Allocate wiphy */
        wdev->wiphy = mem_calloc(sizeof(struct wiphy), 1);/*wiphy_new(&woal_cfg80211_sta_ops, sizeof(moal_private *))*/;
		dev->cfg80211_ops = &woal_cfg80211_sta_ops;
#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
	 }
    else if (bss_type == MLAN_BSS_TYPE_WIFIDIRECT)
		{
        /* Allocate wiphy */
        //wdev->wiphy =
        //    wiphy_new(&woal_cfg80211_wifi_direct_ops, sizeof(moal_private *));
				wdev->wiphy = mem_calloc(sizeof(struct wiphy), 1);/*wiphy_new(&woal_cfg80211_sta_ops, sizeof(moal_private *))*/;
				dev->cfg80211_ops = &woal_cfg80211_wifi_direct_ops;
#endif
#endif
	}else {
        PRINTM(MERROR, "Unexpected bss_type when register cfg80211\n");
        ret = MLAN_STATUS_FAILURE;
        goto err_wdev;
    }

    if (!wdev->wiphy) {
        PRINTM(MERROR, "Could not allocate wiphy device\n");
        ret = MLAN_STATUS_FAILURE;
        goto err_wdev;
    }
    if (bss_type == MLAN_BSS_TYPE_STA) {
        wdev->iftype = NL80211_IFTYPE_STATION;
        wdev->wiphy->interface_modes =
#ifdef UAP_CFG80211
            MBIT(NL80211_IFTYPE_AP) |
#endif /* UAP_CFG80211 */
            MBIT(NL80211_IFTYPE_STATION) | MBIT(NL80211_IFTYPE_ADHOC);
        wdev->wiphy->max_scan_ssids = 10;
    }
#if defined(WIFI_DIRECT_SUPPORT)
#if LINUX_VERSION_CODE >= WIFI_DIRECT_KERNEL_VERSION
    if (bss_type == MLAN_BSS_TYPE_WIFIDIRECT) {
        wdev->iftype = NL80211_IFTYPE_STATION;
        wdev->wiphy->interface_modes = MBIT(NL80211_IFTYPE_STATION) |
            MBIT(NL80211_IFTYPE_P2P_GO) |
            MBIT(NL80211_IFTYPE_P2P_CLIENT) |
            MBIT(NL80211_IFTYPE_ADHOC) | MBIT(NL80211_IFTYPE_AP) | 0;
        wdev->wiphy->max_scan_ssids = 10;
    }
#endif
#endif

    /* Set phy name like net device name */
    //dev_set_name(&wdev->wiphy->dev, dev->name);
	
    /* Make this wiphy known to this driver only */
    //wdev->wiphy->privid = mrvl_wiphy_privid;

    /* Supported bands */
    wdev->wiphy->bands[IEEE80211_BAND_2GHZ] = (struct ieee80211_supported_band*)&cfg80211_band_2ghz;
    if (MLAN_STATUS_SUCCESS ==
        woal_request_get_fw_info(priv, MOAL_CMD_WAIT, &fw_info)) {
        if (fw_info.fw_bands & BAND_A)
            wdev->wiphy->bands[IEEE80211_BAND_5GHZ] = (struct ieee80211_supported_band*)&cfg80211_band_5ghz;
    }
    /* Initialize cipher suits */
    wdev->wiphy->cipher_suites = cfg80211_cipher_suites;
    wdev->wiphy->n_cipher_suites = ARRAY_SIZE(cfg80211_cipher_suites, u32);

    wdev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;

    /* We are using custom domains */
    //wdev->wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
   // wdev->wiphy->flags |=
   //     WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL | WIPHY_FLAG_OFFCHAN_TX;
#endif

    //wdev->wiphy->reg_notifier = woal_cfg80211_reg_notifier;

    /* Set moal_private pointer in wiphy_priv */
    //wdev_priv = wiphy_priv(wdev->wiphy);

    //*(unsigned long *) wdev_priv = (unsigned long) priv;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39) || defined(COMPAT_WIRELESS)
    //set_wiphy_dev(wdev->wiphy, (struct device *) priv->phandle->hotplug_device);
#endif

    //if (wiphy_register(wdev->wiphy) < 0) {
    //    PRINTM(MERROR, "Wiphy device registration failed!\n");
    //    ret = MLAN_STATUS_FAILURE;
    //    goto err_wdev;
    //}

	wdev->wiphy->priv = priv;
    //dev_net_set(dev, wiphy_net(wdev->wiphy));
    dev->ieee80211_ptr = wdev;
    //SET_NETDEV_DEV(dev, wiphy_dev(wdev->wiphy));

    if (ret != MLAN_STATUS_SUCCESS) {
        PRINTM(MERROR, "Wiphy device registration failed!\n");
    } else {
        PRINTM(MINFO, "Successfully registered wiphy device\n");
        LEAVE();
        return ret;
    }

    wiphy_unregister(wdev->wiphy);
  err_wdev:
    dev->ieee80211_ptr = NULL;
    if (wdev) {
        if (wdev->wiphy)
            wiphy_free(wdev->wiphy);
        kfree(wdev);
    }

    LEAVE();
    return ret;
}
#endif

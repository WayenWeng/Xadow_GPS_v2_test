
#include <stdio.h>
#include "vmtype.h"
#include "vmsystem.h" 
#include "vmcmd.h"
#include "vmlog.h"
#include "vmtimer.h"
#include "ResID.h"
#include "vmbt_cm.h"
#include "vmbt_spp.h"
#include "gps_test.h"
#include "ldlcgpio.h"
#include "ldlci2cV2.h"
#include "lgps.h"


#define COMMAND_PORT  1000


// Remote device that this example try to connect
#define BT_NAME "Xadow Phone SPP"
// Standard SPP UUID
#define UUID 0x1101
// Data that this example try to send to remote
#define SPP_DATA "Hello Xadow Phone!\r\n"

VMINT g_tid_switch = -1; // timer id used to post BT process
VMINT g_btcm_hdl = -1; // handle of BT service
VMINT g_btspp_hdl = -1; // handle of SPP service

static vm_bt_cm_bt_address_t g_btspp_addr; // Store BT mac address of BT_NAME device we found during search process
static VMINT g_btspp_min_buf_size;  // size of buffer
static void *g_btspp_buf; // buffer that store SPP received data
static VMINT g_b_server_find; // if BT_NAME device is founded or not duing search process

unsigned char gps_open_flag = 0;
VM_BT_SPP_CONNECTION_ID ble_spp_id = 0;

VM_TIMER_ID_PRECISE sys_timer_id = 0;
unsigned char bt_open = 0;


// SPP servie callback handler
void app_btspp_cb(VM_BT_SPP_EVENT evt, vm_bt_spp_event_cntx_t* param, void* user_data)
{
    vm_bt_spp_event_cntx_t *cntx = (vm_bt_spp_event_cntx_t*)param;
    VMINT ret;
    switch(evt){
        case VM_BT_SPP_EVENT_AUTHORIZE:
        {
            memset(&g_btspp_addr, 0, sizeof(g_btspp_addr));
            ret = vm_bt_spp_get_device_address(cntx->connection_id, &g_btspp_addr);
            ret = vm_bt_spp_accept(cntx->connection_id, g_btspp_buf, g_btspp_min_buf_size, g_btspp_min_buf_size);
        }
        break;

        case VM_BT_SPP_EVENT_READY_TO_WRITE:
        {
        	// write SPP_DATA example string to remote side
        }
        break;

        case VM_BT_SPP_EVENT_READY_TO_READ:
        // read data from remote side and print it out to log
        ret = vm_bt_spp_read(cntx->connection_id, g_btspp_buf, g_btspp_min_buf_size);
        if (ret > 0)
        {
            // log the received data
            ((VMCHAR*)g_btspp_buf)[ret] = 0;
            vm_log_debug("BTSPP vm_btspp_read[%s]", g_btspp_buf);
            ret = vm_bt_spp_write(cntx->connection_id, SPP_DATA, strlen(SPP_DATA));
            ble_spp_id = cntx->connection_id;
            vm_log_info("cntx->connection_id is %d", ble_spp_id);
            vm_log_info("g_btspp_buf[0] is %d", ((VMCHAR*)g_btspp_buf)[0]);
            VMUINT8 bleData = ((VMCHAR*)g_btspp_buf)[0];
            if(bleData == 49)gps_open_flag = 1;
            if(bleData == 48)gps_open_flag = 0;
        }
        break;

        case VM_BT_SPP_EVENT_DISCONNECT:
        // Close SPP
        ret = vm_bt_spp_close(g_btspp_hdl);
        if (g_btspp_buf)
        {
            vm_free(g_btspp_buf);
            g_btspp_buf = NULL;
        }
        g_b_server_find = 0;
        // turn off BT
        ret = vm_bt_cm_switch_off();

        bt_open = 1;
        break;
    }
}

// Init SPP servie and related resources
static void app_btspp_start(void){
    VMINT result;
    VMUINT evt_mask = VM_BT_SPP_EVENT_START	 |
        VM_BT_SPP_EVENT_BIND_FAIL	 |
        VM_BT_SPP_EVENT_AUTHORIZE	 |
        VM_BT_SPP_EVENT_CONNECT	 |
        VM_BT_SPP_EVENT_SCO_CONNECT	|
        VM_BT_SPP_EVENT_READY_TO_WRITE	|
        VM_BT_SPP_EVENT_READY_TO_READ	|
        VM_BT_SPP_EVENT_DISCONNECT	 |
        VM_BT_SPP_EVENT_SCO_DISCONNECT;

    g_btspp_hdl = vm_bt_spp_open(evt_mask, app_btspp_cb, NULL);
    if(g_btspp_hdl < 0)
    {
        return;
    }
    result = vm_bt_spp_set_security_level(g_btspp_hdl, VM_BT_SPP_SECURITY_NONE);
    g_btspp_min_buf_size = vm_bt_spp_get_min_buffer_size();

    g_btspp_buf = vm_calloc(g_btspp_min_buf_size);
    g_btspp_min_buf_size = g_btspp_min_buf_size / 2;
    result = vm_bt_spp_bind(g_btspp_hdl, UUID);
    if(result < 0)
    {
        vm_bt_spp_close(g_btspp_hdl);
        return;
    }
}

// BT servie callback handler
static void app_btcm_cb(VMUINT evt, void * param, void * user_data)
{
    VMINT ret;
    switch (evt){
        case VM_BT_CM_EVENT_ACTIVATE:
        {
            //After activated, continue to scan for devices
            vm_bt_cm_device_info_t dev_info = {0};

            vm_bt_cm_activate_t *active = (vm_bt_cm_activate_t *)param;
            // set BT device host name
            ret = vm_bt_cm_set_host_name((VMUINT8*)BT_NAME);
            // display host info
            ret = vm_bt_cm_get_host_device_info(&dev_info);
            vm_log_debug("BTCM vm_btcm_get_host_dev_info [%d]", ret);
            vm_log_debug("BTCM vm_btcm_get_host_dev_info[%s][0x%02x:%02x:%02x:%02x:%02x:%02x]", dev_info.name,
                ((dev_info.device_address.nap & 0xff00) >> 8),
                (dev_info.device_address.nap & 0x00ff),
                (dev_info.device_address.uap),
                ((dev_info.device_address.lap & 0xff0000) >> 16),
                ((dev_info.device_address.lap & 0x00ff00) >> 8),
                (dev_info.device_address.lap & 0x0000ff));
            // set bt device as visibility
            ret = vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_ON);
            // init SPP services
            app_btspp_start();
            break;
        }

        case VM_BT_CM_EVENT_DEACTIVATE:
        {
            ret = vm_bt_cm_exit(g_btcm_hdl);
            g_btcm_hdl = -1;
            break;
        }

        case VM_BT_CM_EVENT_SET_VISIBILITY:
        {
            vm_bt_cm_device_info_t dev_info = {0};

            vm_bt_cm_set_visibility_t *visi = (vm_bt_cm_set_visibility_t *)param;
            vm_log_debug("BTCM VM_BT_CM_EVENT_SET_VISIBILITY hdl[%d] rst[%d]", visi->handle, visi->result);
            break;
        }

        default:{
            break;
        }
    }
}

// Init BT servie and turn on BT if necessary
static void app_btcm_start(void)
{
    VMINT ret;
    g_btcm_hdl = vm_bt_cm_init(
        app_btcm_cb,
        VM_BT_CM_EVENT_ACTIVATE |
        VM_BT_CM_EVENT_DEACTIVATE |
        VM_BT_CM_EVENT_SET_VISIBILITY |
        VM_BT_CM_EVENT_SET_NAME,
        NULL);

    ret = vm_bt_cm_get_power_status();
    if (VM_BT_CM_POWER_OFF == ret)
    {
    	// Turn on BT if not yet on
        ret = vm_bt_cm_switch_on();
    }
    else if (VM_BT_CM_POWER_ON == ret)
    {
    	// if BT is already on
        vm_bt_cm_device_info_t dev_info = {0};
        // set bt device host name
        ret = vm_bt_cm_set_host_name((VMUINT8*)BT_NAME);
        ret = vm_bt_cm_get_host_device_info(&dev_info);
        vm_log_debug("BTCM vm_btcm_get_host_dev_info[%s][0x%02x:%02x:%02x:%02x:%02x:%02x]", dev_info.name,
            ((dev_info.device_address.nap & 0xff00) >> 8),
            (dev_info.device_address.nap & 0x00ff),
            (dev_info.device_address.uap),
            ((dev_info.device_address.lap & 0xff0000) >> 16),
            ((dev_info.device_address.lap & 0x00ff00) >> 8),
            (dev_info.device_address.lap & 0x0000ff));

        // set bt device as visibility
        ret = vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_ON);

        // init SPP services
        app_btspp_start();
    }
}

static void app_timer_cb(VMINT tid, void* user_data)
{
    vm_log_debug("BTCM app_timer_cb[%d][%d]", tid, g_tid_switch);
    if (tid == g_tid_switch)
    {
        // start BT
        app_btcm_start();

        // stop timer
        vm_timer_delete_precise(g_tid_switch);
        g_tid_switch = -1;
    }
}

void command_callback(vm_cmd_command_t *param, void *user_data)
{
    vm_log_info("cmd = %s", (char*)param->command_buffer);

    if(strcmp("Check",(char*)param->command_buffer) == 0)
    {
    	vm_log_info("gps_check_online is %d", gps_check_online());
    }
    else if(strcmp("GPS",(char*)param->command_buffer) == 0)
    {
    	unsigned char *data;

    	vm_log_info("gps_check_online is %d", gps_check_online());
    	data = gps_get_utc_date_time();
    	vm_log_info("gps_get_utc_date_time is %d-%d-%d,%d:%d:%d", data[0], data[1], data[2], data[3], data[4], data[5]);
    	vm_log_info("gps_get_status is %c", gps_get_status());
    	vm_log_info("gps_get_latitude is %c:%f", gps_get_ns(), gps_get_latitude());
    	vm_log_info("gps_get_longitud is %c:%f", gps_get_ew(), gps_get_longitude());
    	vm_log_info("gps_get_speed is %f", gps_get_speed());
    	vm_log_info("gps_get_course is %f", gps_get_course());
    	vm_log_info("gps_get_position_fix is %c", gps_get_position_fix());
    	vm_log_info("gps_get_sate_in_view is %d", gps_get_sate_in_veiw());
    	vm_log_info("gps_get_sate_used is %d", gps_get_sate_used());
    	vm_log_info("gps_get_altitude is %f", gps_get_altitude());
    	vm_log_info("gps_get_mode is %c", gps_get_mode());
    	vm_log_info("gps_get_mode2 is %c", gps_get_mode2());
    }
}

void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
	vm_log_info("Now is time loop.");

	if(gps_open_flag == 1)
	{
		char temp[64] = {0};
		unsigned char *data;

		data = gps_get_utc_date_time();
		vm_log_info("gps_get_utc_date_time is %d-%d-%d,%d:%d:%d", data[0], data[1], data[2], data[3], data[4], data[5]);
		vm_log_info("gps_get_status is %c", gps_get_status());
		vm_log_info("gps_get_latitude is %c:%f", gps_get_ns(), gps_get_latitude());
		vm_log_info("gps_get_longitud is %c:%f", gps_get_ew(), gps_get_longitude());
		vm_log_info("gps_get_speed is %f", gps_get_speed());
		vm_log_info("gps_get_course is %f", gps_get_course());
		vm_log_info("gps_get_position_fix is %c", gps_get_position_fix());
		vm_log_info("gps_get_sate_in_view is %d", gps_get_sate_in_veiw());
		vm_log_info("gps_get_sate_used is %d", gps_get_sate_used());
		vm_log_info("gps_get_altitude is %f", gps_get_altitude());
		vm_log_info("gps_get_mode is %c", gps_get_mode());
		vm_log_info("gps_get_mode2 is %c", gps_get_mode2());

		sprintf(temp, "GPS get sate in view is %d, sate used is %d.\r\n", gps_get_sate_in_veiw(), gps_get_sate_used());
		vm_bt_spp_write(ble_spp_id, temp, strlen(temp));
	}

	if(bt_open)
	{
		bt_open = 0;
		// wait for 5 seconds for init system
		g_tid_switch = vm_timer_create_precise(5000, app_timer_cb, NULL);
		vm_log_info("g_tid_switch is %d", g_tid_switch);
	}
}

void handle_sysevt(VMINT message, VMINT param)
{
    switch (message)
    {
        case VM_EVENT_CREATE:
			vm_log_info("Open AT Port & Reg Callback");
			vm_cmd_open_port(COMMAND_PORT, command_callback, NULL);
			pinMode(18, INPUT);
			pinMode(13, OUTPUT);
			digitalRead(18);
			digitalWrite(13, HIGH);
            sys_timer_id = vm_timer_create_precise(1000, sys_timer_callback, NULL);
            // wait for 5 seconds for init system
            g_tid_switch = vm_timer_create_precise(5000, app_timer_cb, NULL);
            break;
        case VM_EVENT_PAINT:
            break;
        case VM_EVENT_QUIT:
            break;
    }
}

void vm_main(void) 
{
    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}

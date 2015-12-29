
#include <stdlib.h>
#include "vmtype.h"
#include "vmlog.h"
#include "ldlci2cV2.h"
#include "lgps.h"



unsigned char gps_utc_date_time[GPS_UTC_DATE_TIME_SIZE] = {0};


unsigned char gps_check_online(void)
{
	unsigned char data[GPS_SCAN_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_SCAN_ID);

	for(i=0;i<(GPS_SCAN_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	if(data[5] == GPS_DEVICE_ADDR)return 1;
	else return 0;
}

unsigned char* gps_get_utc_date_time(void)
{
	unsigned char data[GPS_UTC_DATE_TIME_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_UTC_DATE_TIME_ID);

	for(i=0;i<(GPS_UTC_DATE_TIME_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	for(i=0;i<GPS_UTC_DATE_TIME_SIZE;i++)
	gps_utc_date_time[i] = data[i+2];

	return gps_utc_date_time;
}

unsigned char gps_get_status(void)
{
	unsigned char data[GPS_STATUS_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_STATUS_ID);
	for(i=0;i<(GPS_STATUS_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return data[2];
}

float gps_get_latitude(void)
{
	unsigned char data[GPS_LATITUDE_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_LATITUDE_ID);
	for(i=0;i<(GPS_LATITUDE_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return atof(&data[2]);
}

unsigned char gps_get_ns(void)
{
	unsigned char data[GPS_NS_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_NS_ID);
	for(i=0;i<(GPS_NS_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

    if(data[2] == 'N' || data[2] == 'S')return data[2];
    else return data[2] = '-';

}

float gps_get_longitude(void)
{
	unsigned char data[GPS_LONGITUDE_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_LONGITUDE_ID);
	for(i=0;i<(GPS_LONGITUDE_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return atof(&data[2]);
}

unsigned char gps_get_ew(void)
{
	unsigned char data[GPS_EW_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_EW_ID);
	for(i=0;i<(GPS_EW_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	if(data[2] == 'E' || data[2] == 'W')return data[2];
	else return data[2] = '-';
}

float gps_get_speed(void)
{
	unsigned char data[GPS_SPEED_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_SPEED_ID);
	for(i=0;i<(GPS_SPEED_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return atof(&data[2]);
}

float gps_get_course(void)
{
	unsigned char data[GPS_COURSE_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_COURSE_ID);
	for(i=0;i<(GPS_COURSE_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return atof(&data[2]);
}

unsigned char gps_get_position_fix(void)
{
	unsigned char data[GPS_POSITION_FIX_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_POSITION_FIX_ID);
	for(i=0;i<(GPS_POSITION_FIX_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return data[2];
}

unsigned char gps_get_sate_used(void)
{
	unsigned char data[GPS_SATE_USED_SIZE+2];
	unsigned char i;
	unsigned char value;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_SATE_USED_ID);
	for(i=0;i<(GPS_SATE_USED_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	if(data[3] >= '0' && data[3] <= '9')value = (data[3] - '0') * 10;
	else value = 0;
	if(data[2] >= '0' && data[2] <= '9')value += (data[2] - '0');
	else value += 0;

	return value;
}

float gps_get_altitude(void)
{
	unsigned char data[GPS_ALTITUDE_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_ALTITUDE_ID);
	for(i=0;i<(GPS_ALTITUDE_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return atof(&data[2]);
}

unsigned char gps_get_mode(void)
{
	unsigned char data[GPS_MODE_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_MODE_ID);
	for(i=0;i<(GPS_MODE_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return data[2];
}

unsigned char gps_get_mode2(void)
{
	unsigned char data[GPS_MODE2_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_MODE2_ID);
	for(i=0;i<(GPS_MODE2_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return data[2];
}

unsigned char gps_get_sate_in_veiw(void)
{
	unsigned char data[GPS_SATE_IN_VIEW_SIZE+2];
	unsigned char i;

	dlc_i2c_configure(GPS_DEVICE_ADDR, 100);

	dlc_i2c_send_byte(GPS_SATE_IN_VIEW_ID);
	for(i=0;i<(GPS_SATE_IN_VIEW_SIZE+2);i++)
	{
		data[i] = dlc_i2c_receive_byte();
	}

	return data[2];
}


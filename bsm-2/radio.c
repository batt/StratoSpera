#include "radio.h"

#include "measures.h"
#include "status_mgr.h"
#include "gps.h"
#include "testmode.h"
#include "uplink.h"
#include "hadarp.h"

#include <cpu/byteorder.h>
#include <kern/proc.h>
#include <kern/sem.h>

#include <drv/timer.h>
#include <net/afsk.h>
#include <net/ax25.h>

#include <mware/config.h>
#include <mware/find_token.h>

#include <sec/hash/md5.h>
#include <sec/mac/hmac.h>

#include <cfg/compiler.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define LOG_LEVEL LOG_LVL_INFO
#include <cfg/log.h>
#include "logging.h"

static Afsk afsk;
static AX25Ctx ax25;
static bool radio_initialized;

static void radio_reload(void);

DECLARE_CONF(radio, radio_reload,
	CONF_INT(aprs_interval, 1, 3600, 15),
	CONF_STRING(send_call, 7, "STSP"),
	CONF_STRING(sign_key, 16, "<Strat0Sp3r4>"),
	CONF_BOOL(check_auth, true)
);

static HmacContext hmac;
static MD5_Context md5;

/*
 * BSM-2 Message format:
 *
 * }}>[SIGN]>[SEQ]>[PAYLOAD]
 *
 * [SIGN]:
 * Hex representation of the signature (hmac-md5). It is only 16 bits long,
 * the low byte is taken from byte 7 of the md5 digest, while the high part uses
 * byte 13.
 * The signature is computed on all the bytes following the first '>' separator:
 * [SEQ], the second '>' separator and the [PAYLOAD].
 *
 * [SEQ]:
 * Hex representation of message sequence number. This number must be
 * monotonically increasing.
 *
 * [PAYLOAD]:
 * The payload which is passed to the uplink parser module.
 */
#define HDR_STR "{{>"
#define HDR_LEN (sizeof(HDR_STR) - 1)
static uint32_t last_seq = 0;

static void radio_uplinkDecoder(const void *msg, size_t len)
{
	if (len > HDR_LEN && memcmp(msg, HDR_STR, HDR_LEN) == 0)
	{
		LOG_INFO("BSM-2 uplink header detected\n");
		const char *buf = (const char *)msg + HDR_LEN;
		const char *end = buf + len - HDR_LEN;
		char msg_num[9];

		buf = find_token(buf, end - buf, msg_num, sizeof(msg_num), ">");
		uint16_t sign = strtoul(msg_num, NULL, 16);
		LOG_INFO("Received signature: [%s] = %04X\n", msg_num, sign);
		mac_begin(&hmac.m);
		mac_update(&hmac.m, buf, end - buf);
		uint8_t *s = mac_final(&hmac.m);
		uint16_t lsign = s[7] | (s[13] << 8);
		LOG_INFO("Computed signature: %04X\n", lsign);

		buf = find_token(buf, end - buf, msg_num, sizeof(msg_num), ">");
		uint32_t seqn = strtoul(msg_num, NULL, 16);
		LOG_INFO("Received seq number: [%s] = %lX\n", msg_num, seqn);

		if (sign == lsign || !check_auth)
		{
			bool res = false;
			LOG_INFO("Signature check OK\n");
			if (seqn > last_seq || !check_auth)
			{
				LOG_INFO("Seq number check OK\n");
				res = uplink_parse(buf, end - buf);
				last_seq = seqn;
				radio_printf(">%lX:%s\n", seqn, res ? "OK" : "ERR");
			}
			else
				LOG_INFO("Seq number check FAIL\n");
		}
		else
			LOG_INFO("Signature check FAIL\n");
	}
	else
		LOG_INFO("Unknown message format received\n");
}

static void ax25_log(struct AX25Msg *msg)
{
	if (!testmode())
		logging_msg("RADIO:%.6s-%d>%.6s-%d:%.*s\n",
			msg->src.call, msg->src.ssid,
			msg->dst.call, msg->dst.ssid,
			msg->len, msg->info);
	else
		kprintf("RADIO:%.6s-%d>%.6s-%d:%.*s\n",
			msg->src.call, msg->src.ssid,
			msg->dst.call, msg->dst.ssid,
			msg->len, msg->info);

	radio_uplinkDecoder(msg->info, msg->len);
}

static AX25Call ax25_path[2]=
{
	AX25_CALL("APZBRT", 0),
};

static Semaphore radio_sem;
static char radio_msg[128];

#define RADIO_PAUSE_TIME (5 * 60 * 1000) //5min
static ticks_t radio_pause_time = 0;

static void radio_send(char *buf, size_t len)
{
	if (radio_pause_time && timer_clock() - radio_pause_time < ms_to_ticks(RADIO_PAUSE_TIME))
	{
		LOG_INFO("Radio pause: message discarded\n");
		return;
	}

	radio_pause_time = 0;
	ax25_sendVia(&ax25, ax25_path, countof(ax25_path), buf, len);
}

void radio_time(char *time_str, size_t size)
{
	static time_t prev_t;

	sem_obtain(&radio_sem);
	time_t tim = gps_time();
	/* Avoid sending two messages with the same timestamp */
	if (tim <= prev_t)
		tim = prev_t + 1;

	struct tm t;
	gmtime_r(&tim, &t);
	prev_t = tim;

	snprintf(time_str, size, "%02d%02d%02d", t.tm_hour, t.tm_min, t.tm_sec);
	time_str[size - 1] = '\0';
	sem_release(&radio_sem);
}

int radio_printf(const char * fmt, ...)
{
	int result;
	va_list ap;
	va_start(ap, fmt);
	logging_vmsg(fmt, ap);
	va_end(ap);

	if (!radio_initialized)
	{
		LOG_WARN("Radio not yet initialized\n");
		return EOF;
	}

	char time[7];
	radio_time(time, sizeof(time));

	sem_obtain(&radio_sem);
	sprintf(radio_msg, ">%sh", time);

	va_start(ap, fmt);
	result = vsnprintf(&radio_msg[8], sizeof(radio_msg) - 8, fmt, ap);
	va_end(ap);

	radio_send(radio_msg, strnlen(radio_msg, sizeof(radio_msg)));
	sem_release(&radio_sem);
	return result;
}

void radio_sendTelemetry(void)
{
	sem_obtain(&radio_sem);
	measures_aprsFormat(radio_msg, sizeof(radio_msg));
	radio_send(radio_msg, strnlen(radio_msg, sizeof(radio_msg)));
	sem_release(&radio_sem);
}

#define STARTUP_SETUP_TIME  (3 * 60 * 1000) // 3 minutes
#define STARTUP_SETUP_DELAY ms_to_ticks(10 * 1000) // 10 seconds

static void NORETURN radio_process(void)
{
	ticks_t start = timer_clock();
	ticks_t delay;

	while (1)
	{
		ax25_poll(&ax25);
		timer_delay(100);

		if (testmode())
			continue;

		if (status_missionTime() < STARTUP_SETUP_TIME)
		{
			/* At startup we need to test communications, so use a short delay */
			delay = STARTUP_SETUP_DELAY;
		}
		else if (status_currStatus() == BSM2_NOFIX
			  || status_currStatus() == BSM2_GROUND_WAIT)
		{
			/* When we are at ground or with no fix, relax timings in order to
			 * save radio battery */
			delay = ms_to_ticks(aprs_interval * 4000);
		}
		else
			delay = ms_to_ticks(aprs_interval * 1000);

		ticks_t now = timer_clock();
		if (now - start > delay)
		{
			/* Avoid sending multiple messages the radio
			 * is busy for a long time */
			while (now - start > delay)
				start += delay;
			radio_sendTelemetry();
		}
	}
}

static void radio_reload(void)
{
	LOG_INFO("Setting radio configuration\n");
	LOG_INFO(" APRS messages interval %d seconds\n", aprs_interval);
	LOG_INFO(" Source CALL [%.6s]\n", send_call);
	LOG_INFO(" Signature check for uplink radio commands:%s\n", check_auth ? "ENABLED" : "DISABLED");
	LOG_INFO(" Uplink radio commands signature key [%s]\n", sign_key);
	memset(ax25_path[1].call, 0, sizeof(ax25_path[1].call));
	strncpy(ax25_path[1].call, send_call, sizeof(ax25_path[1].call));
	ax25_path[1].ssid = 0;
	if (!check_auth)
		last_seq = 0;
	mac_set_key(&hmac.m, (const uint8_t *)sign_key, strlen(sign_key));
}

static bool radio_ping(long l)
{
	radio_printf("PONG %ld\n", l);
	return true;
}

/*
 * Pause the radio for RADIO_PAUSE_TIME.
 *
 * Useful if you want to test different radio devices on board and
 * you need the main one to be disabled.
 * For RADIO_PAUSE_TIME milliseconds all radio messages will be discarded.
 *
 * NOTE: you can call radio_pause repeatedly, each time the timeout is reset or
 *  you can call radio_resume to cancel the pause in advance.
 */
static bool cmd_radio_pause(long l)
{
	(void)l;
	radio_printf("Pausing radio for 5min\n");
	radio_pause_time = timer_clock();
	hadarp_wakePolifemo();
	return true;
}

/*
 * Call radio_resume to cancel a pause in advance.
 * If there is no pause this function will have no effect.
 */
static bool cmd_radio_resume(long l)
{
	(void)l;
	radio_pause_time = 0;
	return true;
}

void radio_init(void)
{
	sem_init(&radio_sem);

	MD5_init(&md5);
	hmac_init(&hmac, &md5.h);

	config_register(&radio);
	config_load(&radio);

	afsk_init(&afsk, ADC_RADIO_CH, 0);
	ax25_init(&ax25, &afsk.fd, ax25_log);
	radio_initialized = true;
	uplink_registerCmd("ping", radio_ping);
	uplink_registerCmd("radio_pause", cmd_radio_pause);
	uplink_registerCmd("radio_resume", cmd_radio_resume);

	Process *p = proc_new(radio_process, NULL, KERN_MINSTACKSIZE * 6, NULL);
	ASSERT(p);
}

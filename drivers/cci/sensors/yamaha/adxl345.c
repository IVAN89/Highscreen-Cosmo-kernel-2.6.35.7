/*
 * ADXL345 accelerometer driver
 *
 * Copyright (c) 2010 Yamaha Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */


#include <linux/err.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/slab.h>

#define ADXL345_VERSION "1.1.0"
#define ADXL345_NAME    "adxl345"

/* for debugging */
#define DEBUG 0
#define DEBUG_DELAY 0
#define TRACE_FUNC() pr_debug(ADXL345_NAME ": <trace> %s()\n", __FUNCTION__)

/*
 * Default parameters
 */
#define ADXL345_DEFAULT_DELAY               100
#define ADXL345_MAX_DELAY                   2000

/*
 * Registers
 */
#define ADXL345_DEVID_REG                   0x00
#define ADXL345_DEVID                       0xe6 //ADXL346 DEVID

#define ADXL345_ACC_REG                     0x32

#define ADXL345_SOFT_RESET_REG              0x1d
#define ADXL345_SOFT_RESET_MASK             0x80
#define ADXL345_SOFT_RESET_SHIFT            7

#define ADXL345_POWER_CONTROL_REG           0x2d
#define ADXL345_POWER_CONTROL_MASK          0x08
#define ADXL345_POWER_CONTROL_SHIFT         3

#define ADXL345_RANGE_REG                   0x31
#define ADXL345_RANGE_MASK                  0x03
#define ADXL345_RANGE_SHIFT                 0
#define ADXL345_RANGE_16G                   3
#define ADXL345_RANGE_8G                    2
#define ADXL345_RANGE_4G                    1
#define ADXL345_RANGE_2G                    0

#define ADXL345_BANDWIDTH_REG               0x2c
#define ADXL345_BANDWIDTH_MASK              0x0f
#define ADXL345_BANDWIDTH_SHIFT             0
#define ADXL345_BANDWIDTH_800HZ             13
#define ADXL345_BANDWIDTH_400HZ             12
#define ADXL345_BANDWIDTH_200HZ             11
#define ADXL345_BANDWIDTH_100HZ             10
#define ADXL345_BANDWIDTH_50HZ               9
#define ADXL345_BANDWIDTH_25HZ               8
#define ADXL345_BANDWIDTH_12HZ               7
#define ADXL345_BANDWIDTH_6HZ                6
#define ADXL345_BANDWIDTH_3HZ                5
#define ADXL345_BANDWIDTH_2HZ                4
#define ADXL345_BANDWIDTH_1HZ                3

/*
 * Acceleration measurement
 */
#define ADXL345_RESOLUTION                   256
#define GRAVITY_EARTH_RES                    43585

/* ABS axes parameter range [um/s^2] (for input event) */
#define GRAVITY_EARTH                        9806550
#define ABSMIN_2G                            (-GRAVITY_EARTH * 2)
#define ABSMAX_2G                            (GRAVITY_EARTH * 2)

#if DEBUG
#define DEVID                                0x00
#define THRESH_TAP                           0x1d
#define OFSX                                 0x1e
#define OFSY                                 0x1f
#define OFSZ                                 0x20
#define DUR                                  0x21
#define LATENT                               0x22
#define WINDOW                               0x23
#define THRESH_ACT                           0x24
#define THRESH_INACT                         0x25
#define TIME_INACT                           0x26
#define ACT_INACT_CTL                        0x27
#define THRESH_FF                            0x28
#define TIME_FF                              0x29
#define TAP_AXES                             0x2a
#define ACT_TAP_STATUS                       0x2b
#define BW_RATE                              0x2c
#define POWER_CTL                            0x2d
#define INT_ENABLE                           0x2e
#define INT_MAP                              0x2f
#define INT_SOURCE                           0x30
#define DATA_FORMAT                          0x31
#define DATAX0                               0x32
#define DATAX1                               0x33
#define DATAY0                               0x34
#define DATAY1                               0x35
#define DATAZ0                               0x36
#define DATAZ1                               0x37
#define FIFO_CTL                             0x38
#define FIFO_STATUS                          0x39
#endif /* DEBUG */

struct acceleration {
    int x;
    int y;
    int z;
};

/*
 * Output data rate
 */
struct adxl345_odr {
        unsigned long delay;            /* min delay (msec) in the range of ODR */
        u8 odr;                         /* bandwidth register value */
};

static const struct adxl345_odr adxl345_odr_table[] = {
    {1,     ADXL345_BANDWIDTH_800HZ},
    {2,     ADXL345_BANDWIDTH_400HZ},
    {5,     ADXL345_BANDWIDTH_200HZ},
    {10,    ADXL345_BANDWIDTH_100HZ},
    {20,    ADXL345_BANDWIDTH_50HZ},
    {40,    ADXL345_BANDWIDTH_25HZ},
    {80,    ADXL345_BANDWIDTH_12HZ}, /* 12.5Hz   */
    {160,   ADXL345_BANDWIDTH_6HZ},  /*  6.25Hz  */
    {320,   ADXL345_BANDWIDTH_3HZ},  /*  3.125Hz */
    {640,   ADXL345_BANDWIDTH_2HZ},  /*  1.563Hz */
    {1280,  ADXL345_BANDWIDTH_1HZ},  /*  0.782Hz */
};

/*
 * Transformation matrix for chip mounting position
 */
static const int adxl345_position_map[][3][3] = {
    {{ 0, -1,  0}, { 1,  0,  0}, { 0,  0,  1}}, /* top/upper-left */
    {{ 1,  0,  0}, { 0,  1,  0}, { 0,  0,  1}}, /* top/upper-right */
    {{ 0,  1,  0}, {-1,  0,  0}, { 0,  0,  1}}, /* top/lower-right */
    {{-1,  0,  0}, { 0, -1,  0}, { 0,  0,  1}}, /* top/lower-left */
    {{ 0,  1,  0}, { 1,  0,  0}, { 0,  0, -1}}, /* bottom/upper-left */
    {{-1,  0,  0}, { 0,  1,  0}, { 0,  0, -1}}, /* bottom/upper-right */
    {{ 0, -1,  0}, {-1,  0,  0}, { 0,  0, -1}}, /* bottom/lower-right */
    {{ 1,  0,  0}, { 0, -1,  0}, { 0,  0, -1}}, /* bottom/upper-left */
};
/*
 * driver private data
 */
struct adxl345_data {
    atomic_t enable;                /* attribute value */
    atomic_t delay;                 /* attribute value */
    atomic_t position;              /* attribute value */
    struct acceleration last;       /* last measured data */
    struct mutex enable_mutex;
    struct mutex data_mutex;
    struct i2c_client *client;
    struct input_dev *input;
    struct delayed_work work;
#if DEBUG
    int suspend;
#endif
};

#define delay_to_jiffies(d) ((d)?msecs_to_jiffies(d):1)
#define actual_delay(d)     (jiffies_to_msecs(delay_to_jiffies(d)))

/* register access functions */
#define adxl345_read_bits(p,r) \
    ((i2c_smbus_read_byte_data((p)->client, r##_REG) & r##_MASK) >> r##_SHIFT)
#define adxl345_update_bits(p,r,v) \
    i2c_smbus_write_byte_data((p)->client, r##_REG, \
                              ((i2c_smbus_read_byte_data((p)->client,r##_REG) & ~r##_MASK) | ((v) << r##_SHIFT)))
/*
 * Device dependant operations
 */
static int adxl345_power_up(struct adxl345_data *adxl345)
{
    adxl345_update_bits(adxl345, ADXL345_POWER_CONTROL, 1);

    return 0;
}

static int adxl345_power_down(struct adxl345_data *adxl345)
{
    adxl345_update_bits(adxl345, ADXL345_POWER_CONTROL, 0);

    return 0;
}

static int adxl345_hw_init(struct adxl345_data *adxl345)
{
    adxl345_power_down(adxl345);

    adxl345_update_bits(adxl345, ADXL345_RANGE, ADXL345_RANGE_2G);

    return 0;
}

static int adxl345_get_enable(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);

    return atomic_read(&adxl345->enable);
}

static void adxl345_set_enable(struct device *dev, int enable)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);
    int delay = atomic_read(&adxl345->delay);

    mutex_lock(&adxl345->enable_mutex);

    if (enable) {                   /* enable if state will be changed */
        if (!atomic_cmpxchg(&adxl345->enable, 0, 1)) {
            adxl345_power_up(adxl345);
            schedule_delayed_work(&adxl345->work, delay_to_jiffies(delay) + 1);
        }
    } else {                        /* disable if state will be changed */
        if (atomic_cmpxchg(&adxl345->enable, 1, 0)) {
            cancel_delayed_work_sync(&adxl345->work);
            adxl345_power_down(adxl345);
        }
    }
    atomic_set(&adxl345->enable, enable);

    mutex_unlock(&adxl345->enable_mutex);
}

static int adxl345_get_delay(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);

    return atomic_read(&adxl345->delay);
}

static void adxl345_set_delay(struct device *dev, int delay)
{

    struct i2c_client *client = to_i2c_client(dev);
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);
    u8 odr;
    int i;

    /* determine optimum ODR */
    for (i = 1; (i < ARRAY_SIZE(adxl345_odr_table)) &&
             (actual_delay(delay) >= adxl345_odr_table[i].delay); i++)
        ;
    odr = adxl345_odr_table[i-1].odr;
    atomic_set(&adxl345->delay, delay);

    mutex_lock(&adxl345->enable_mutex);

    if (adxl345_get_enable(dev)) {
        cancel_delayed_work_sync(&adxl345->work);
        adxl345_update_bits(adxl345, ADXL345_BANDWIDTH, odr);
        schedule_delayed_work(&adxl345->work, delay_to_jiffies(delay) + 1);
    } else {
        adxl345_power_up(adxl345);
        adxl345_update_bits(adxl345, ADXL345_BANDWIDTH, odr);
        adxl345_power_down(adxl345);
    }

    mutex_unlock(&adxl345->enable_mutex);
}

static int adxl345_get_position(struct device *dev)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);

    return atomic_read(&adxl345->position);
}

static void adxl345_set_position(struct device *dev, int position)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);

    atomic_set(&adxl345->position, position);
}

static int adxl345_measure(struct adxl345_data *adxl345, struct acceleration *accel)
{
    struct i2c_client *client = adxl345->client;
    u8 buf[6];
    int raw[3], data[3];
    int pos = atomic_read(&adxl345->position);
    long long g;
    int i, j;
#if DEBUG_DELAY
    struct timespec t;
#endif

#if DEBUG_DELAY
    getnstimeofday(&t);
#endif

    /* read acceleration data */
    if (i2c_smbus_read_i2c_block_data(client, ADXL345_ACC_REG, 6, buf) != 6) {
        dev_err(&client->dev,
                "I2C block read error: addr=0x%02x, len=%d\n",
                ADXL345_ACC_REG, 6);
            raw[0] = 0;
            raw[1] = 0;
            raw[2] = 0;
    } else {
        raw[0] = (int) (s16)(((buf[1]) << 8) | buf[0]);
        raw[1] = (int) (s16)(((buf[3]) << 8) | buf[2]);
        raw[2] = (int) (s16)(((buf[5]) << 8) | buf[4]);
    }

    /* for X, Y, Z axis */
    for (i = 0; i < 3; i++) {
        /* coordinate transformation */
        data[i] = 0;
        for (j = 0; j < 3; j++) {
            data[i] += raw[j] * adxl345_position_map[pos][i][j];
        }
        /* normalization */
        g = (long long)data[i] * GRAVITY_EARTH / ADXL345_RESOLUTION;
        data[i] = g;
    }

    dev_dbg(&client->dev, "raw(%5d,%5d,%5d) => norm(%8d,%8d,%8d)\n",
            raw[0], raw[1], raw[2], data[0], data[1], data[2]);

#if DEBUG_DELAY
    dev_info(&client->dev, "%ld.%lds:raw(%5d,%5d,%5d) => norm(%8d,%8d,%8d)\n", t.tv_sec, t.tv_nsec,
             raw[0], raw[1], raw[2], data[0], data[1], data[2]);
#endif

    accel->x = data[0];
    accel->y = data[1];
    accel->z = data[2];

    return 0;
}

static void adxl345_work_func(struct work_struct *work)
{
    struct adxl345_data *adxl345 = container_of((struct delayed_work *)work, struct adxl345_data, work);
    struct acceleration accel;
    unsigned long delay = delay_to_jiffies(atomic_read(&adxl345->delay));

    adxl345_measure(adxl345, &accel);

    input_report_abs(adxl345->input, ABS_X, accel.x);
    input_report_abs(adxl345->input, ABS_Y, accel.y);
    input_report_abs(adxl345->input, ABS_Z, accel.z);
    input_sync(adxl345->input);

    mutex_lock(&adxl345->data_mutex);
    adxl345->last = accel;
    mutex_unlock(&adxl345->data_mutex);

    schedule_delayed_work(&adxl345->work, delay);
}

/*
 * Input device interface
 */
static int adxl345_input_init(struct adxl345_data *adxl345)
{
    struct input_dev *dev;
    int err;

    dev = input_allocate_device();
    if (!dev) {
        return -ENOMEM;
    }
    dev->name = "accelerometer";
    dev->id.bustype = BUS_I2C;

    input_set_capability(dev, EV_ABS, ABS_MISC);
    input_set_abs_params(dev, ABS_X, ABSMIN_2G, ABSMAX_2G, 0, 0);
    input_set_abs_params(dev, ABS_Y, ABSMIN_2G, ABSMAX_2G, 0, 0);
    input_set_abs_params(dev, ABS_Z, ABSMIN_2G, ABSMAX_2G, 0, 0);
    input_set_drvdata(dev, adxl345);

    err = input_register_device(dev);
    if (err < 0) {
        input_free_device(dev);
        return err;
    }
    adxl345->input = dev;

    return 0;
}

static void adxl345_input_fini(struct adxl345_data *adxl345)
{
    struct input_dev *dev = adxl345->input;

    input_unregister_device(dev);
    input_free_device(dev);
}

/*
 * sysfs device attributes
 */
static ssize_t adxl345_enable_show(struct device *dev,
                                 struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", adxl345_get_enable(dev));
}

static ssize_t adxl345_enable_store(struct device *dev,
                                  struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    unsigned long enable = simple_strtoul(buf, NULL, 10);

    if ((enable == 0) || (enable == 1)) {
        adxl345_set_enable(dev, enable);
    }

    return count;
}

static ssize_t adxl345_delay_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", adxl345_get_delay(dev));
}

static ssize_t adxl345_delay_store(struct device *dev,
                                 struct device_attribute *attr,
                                 const char *buf, size_t count)
{
    unsigned long delay = simple_strtoul(buf, NULL, 10);

    if (delay > ADXL345_MAX_DELAY) {
        delay = ADXL345_MAX_DELAY;
    }

    adxl345_set_delay(dev, delay);

    return count;
}

static ssize_t adxl345_position_show(struct device *dev,
                                   struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", adxl345_get_position(dev));
}

static ssize_t adxl345_position_store(struct device *dev,
                                    struct device_attribute *attr,
                                    const char *buf, size_t count)
{
    unsigned long position;

    position = simple_strtoul(buf, NULL,10);
    if ((position >= 0) && (position <= 7)) {
        adxl345_set_position(dev, position);
    }

    return count;
}

static ssize_t adxl345_wake_store(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf, size_t count)
{
    struct input_dev *input = to_input_dev(dev);
    static atomic_t serial = ATOMIC_INIT(0);

    input_report_abs(input, ABS_MISC, atomic_inc_return(&serial));

    return count;
}

static ssize_t adxl345_data_show(struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    struct input_dev *input = to_input_dev(dev);
    struct adxl345_data *adxl345 = input_get_drvdata(input);
    struct acceleration accel;

    mutex_lock(&adxl345->data_mutex);
    accel = adxl345->last;
    mutex_unlock(&adxl345->data_mutex);

    return sprintf(buf, "%d %d %d\n", accel.x, accel.y, accel.z);
}

#if DEBUG
static ssize_t adxl345_debug_reg_show(struct device *dev,
                                     struct device_attribute *attr, char *buf)
{
    struct input_dev *input = to_input_dev(dev);
    struct adxl345_data *adxl345 = input_get_drvdata(input);
    struct i2c_client *client = adxl345->client;
    ssize_t count = 0;
    u8 reg;

    reg = i2c_smbus_read_byte_data(client, DEVID);
    count += sprintf(&buf[count], "%02x: %d\n", DEVID, reg);

    reg = i2c_smbus_read_byte_data(client, THRESH_TAP);
    count += sprintf(&buf[count], "%02x: %d\n", THRESH_TAP, reg);

    reg = i2c_smbus_read_byte_data(client, OFSX);
    count += sprintf(&buf[count], "%02x: %d\n", OFSX, reg);

    reg = i2c_smbus_read_byte_data(client, OFSY);
    count += sprintf(&buf[count], "%02x: %d\n", OFSY, reg);

    reg = i2c_smbus_read_byte_data(client, OFSZ);
    count += sprintf(&buf[count], "%02x: %d\n", OFSZ, reg);

    reg = i2c_smbus_read_byte_data(client, DUR);
    count += sprintf(&buf[count], "%02x: %d\n", DUR, reg);

    reg = i2c_smbus_read_byte_data(client, LATENT);
    count += sprintf(&buf[count], "%02x: %d\n", LATENT, reg);

    reg = i2c_smbus_read_byte_data(client, WINDOW);
    count += sprintf(&buf[count], "%02x: %d\n", WINDOW, reg);

    reg = i2c_smbus_read_byte_data(client, THRESH_ACT);
    count += sprintf(&buf[count], "%02x: %d\n", THRESH_ACT, reg);

    reg = i2c_smbus_read_byte_data(client, THRESH_INACT);
    count += sprintf(&buf[count], "%02x: %d\n", THRESH_INACT, reg);

    reg = i2c_smbus_read_byte_data(client, TIME_INACT);
    count += sprintf(&buf[count], "%02x: %d\n", TIME_INACT, reg);

    reg = i2c_smbus_read_byte_data(client, ACT_INACT_CTL);
    count += sprintf(&buf[count], "%02x: %d\n", ACT_INACT_CTL, reg);

    reg = i2c_smbus_read_byte_data(client, THRESH_FF);
    count += sprintf(&buf[count], "%02x: %d\n", THRESH_FF, reg);

    reg = i2c_smbus_read_byte_data(client, TIME_FF);
    count += sprintf(&buf[count], "%02x: %d\n", TIME_FF, reg);

    reg = i2c_smbus_read_byte_data(client, TAP_AXES);
    count += sprintf(&buf[count], "%02x: %d\n", TAP_AXES, reg);

    reg = i2c_smbus_read_byte_data(client, ACT_TAP_STATUS);
    count += sprintf(&buf[count], "%02x: %d\n", ACT_TAP_STATUS, reg);

    reg = i2c_smbus_read_byte_data(client, BW_RATE);
    count += sprintf(&buf[count], "%02x: %d\n", BW_RATE, reg);

    reg = i2c_smbus_read_byte_data(client, POWER_CTL);
    count += sprintf(&buf[count], "%02x: %d\n", POWER_CTL, reg);

    reg = i2c_smbus_read_byte_data(client, INT_ENABLE);
    count += sprintf(&buf[count], "%02x: %d\n", INT_ENABLE, reg);

    reg = i2c_smbus_read_byte_data(client, INT_MAP);
    count += sprintf(&buf[count], "%02x: %d\n", INT_MAP, reg);

    reg = i2c_smbus_read_byte_data(client, INT_SOURCE);
    count += sprintf(&buf[count], "%02x: %d\n", INT_SOURCE, reg);

    reg = i2c_smbus_read_byte_data(client, DATA_FORMAT);
    count += sprintf(&buf[count], "%02x: %d\n", DATA_FORMAT, reg);

    reg = i2c_smbus_read_byte_data(client, DATAX0);
    count += sprintf(&buf[count], "%02x: %d\n", DATAX0, reg);

    reg = i2c_smbus_read_byte_data(client, DATAX1);
    count += sprintf(&buf[count], "%02x: %d\n", DATAX1, reg);

    reg = i2c_smbus_read_byte_data(client, DATAY0);
    count += sprintf(&buf[count], "%02x: %d\n", DATAY0, reg);

    reg = i2c_smbus_read_byte_data(client, DATAY1);
    count += sprintf(&buf[count], "%02x: %d\n", DATAY1, reg);

    reg = i2c_smbus_read_byte_data(client, DATAZ0);
    count += sprintf(&buf[count], "%02x: %d\n", DATAZ0, reg);

    reg = i2c_smbus_read_byte_data(client, DATAZ1);
    count += sprintf(&buf[count], "%02x: %d\n", DATAZ1, reg);

    reg = i2c_smbus_read_byte_data(client, FIFO_CTL);
    count += sprintf(&buf[count], "%02x: %d\n", FIFO_CTL, reg);

    reg = i2c_smbus_read_byte_data(client, FIFO_STATUS);
    count += sprintf(&buf[count], "%02x: %d\n", FIFO_STATUS, reg);

    return count;
}

static int adxl345_suspend(struct i2c_client *client, pm_message_t mesg);
static int adxl345_resume(struct i2c_client *client);

static ssize_t adxl345_debug_suspend_show(struct device *dev,
                                         struct device_attribute *attr, char *buf)
{
    struct input_dev *input = to_input_dev(dev);
    struct adxl345_data *adxl345 = input_get_drvdata(input);

    return sprintf(buf, "%d\n", adxl345->suspend);
}

static ssize_t adxl345_debug_suspend_store(struct device *dev,
                                          struct device_attribute *attr,
                                          const char *buf, size_t count)
{
    struct input_dev *input = to_input_dev(dev);
    struct adxl345_data *adxl345 = input_get_drvdata(input);
    struct i2c_client *client = adxl345->client;
    unsigned long suspend = simple_strtoul(buf, NULL, 10);

    if (suspend) {
        pm_message_t msg;
        adxl345_suspend(client, msg);
    } else {
        adxl345_resume(client);
    }

    return count;
}
#endif /* DEBUG */

static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
                   adxl345_enable_show, adxl345_enable_store);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
                   adxl345_delay_show, adxl345_delay_store);
static DEVICE_ATTR(position, S_IRUGO|S_IWUSR,
                   adxl345_position_show, adxl345_position_store);
static DEVICE_ATTR(wake, S_IWUSR|S_IWGRP, NULL, adxl345_wake_store);
static DEVICE_ATTR(data, S_IRUGO,
                   adxl345_data_show, NULL);

#if DEBUG
static DEVICE_ATTR(debug_reg, S_IRUGO,
                   adxl345_debug_reg_show, NULL);
static DEVICE_ATTR(debug_suspend, S_IRUGO|S_IWUSR,
                   adxl345_debug_suspend_show, adxl345_debug_suspend_store);
#endif /* DEBUG */

static struct attribute *adxl345_attributes[] = {
    &dev_attr_enable.attr,
    &dev_attr_delay.attr,
    &dev_attr_position.attr,
    &dev_attr_wake.attr,
    &dev_attr_data.attr,
#if DEBUG
    &dev_attr_debug_reg.attr,
    &dev_attr_debug_suspend.attr,
#endif /* DEBUG */
    NULL
};

static struct attribute_group adxl345_attribute_group = {
    .attrs = adxl345_attributes
};

/*
 * I2C client
 */
static int adxl345_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    int id;

    id = i2c_smbus_read_byte_data(client, ADXL345_DEVID_REG);
    if (id != ADXL345_DEVID)
        return -ENODEV;

    return 0;
}


static int adxl345_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct adxl345_data *adxl345;
    int err;

    TRACE_FUNC();

    /* setup private data */
    adxl345 = kzalloc(sizeof(struct adxl345_data), GFP_KERNEL);
    if (!adxl345) {
        err = -ENOMEM;
        goto error_0;
    }

    mutex_init(&adxl345->enable_mutex);
    mutex_init(&adxl345->data_mutex);

    /* setup i2c client */
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        err = -ENODEV;
        goto error_1;
    }
    i2c_set_clientdata(client, adxl345);
    adxl345->client = client;

    /* detect and init hardware */
    if ((err = adxl345_detect(client, NULL))) {
        goto error_1;
    }
    dev_info(&client->dev, "%s found\n", id->name);

    adxl345_hw_init(adxl345);
    adxl345_set_delay(&client->dev, ADXL345_DEFAULT_DELAY);
    adxl345_set_position(&client->dev, CONFIG_INPUT_ADXL345_POSITION);

    /* setup driver interfaces */
    INIT_DELAYED_WORK(&adxl345->work, adxl345_work_func);

    err = adxl345_input_init(adxl345);
    if (err < 0) {
        goto error_1;
    }

    err = sysfs_create_group(&adxl345->input->dev.kobj, &adxl345_attribute_group);
    if (err < 0) {
        goto error_2;
    }

    return 0;

error_2:
    adxl345_input_fini(adxl345);
error_1:
    kfree(adxl345);
error_0:
    return err;
}

static int adxl345_remove(struct i2c_client *client)
{
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);

    adxl345_set_enable(&client->dev, 0);

    sysfs_remove_group(&adxl345->input->dev.kobj, &adxl345_attribute_group);
    adxl345_input_fini(adxl345);

    kfree(adxl345);

    return 0;
}

static int adxl345_suspend(struct i2c_client *client, pm_message_t mesg)
{
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);

    mutex_lock(&adxl345->enable_mutex);

    if (adxl345_get_enable(&client->dev)) {
        cancel_delayed_work_sync(&adxl345->work);
        adxl345_power_down(adxl345);
    }

#if DEBUG
    adxl345->suspend = 1;
#endif

    mutex_unlock(&adxl345->enable_mutex);

    return 0;
}

static int adxl345_resume(struct i2c_client *client)
{
    struct adxl345_data *adxl345 = i2c_get_clientdata(client);
    int delay = atomic_read(&adxl345->delay);

    adxl345_hw_init(adxl345);
    adxl345_set_delay(&client->dev, delay);

    mutex_lock(&adxl345->enable_mutex);

    if (adxl345_get_enable(&client->dev)) {
        adxl345_power_up(adxl345);
        schedule_delayed_work(&adxl345->work, delay_to_jiffies(delay) + 1);
    }

#if DEBUG
    adxl345->suspend = 0;
#endif

    mutex_unlock(&adxl345->enable_mutex);

    return 0;
}

static const struct i2c_device_id adxl345_id[] = {
    {ADXL345_NAME, 0},
    {},
};

MODULE_DEVICE_TABLE(i2c, adxl345_id);

static struct i2c_driver adxl345_driver = {
    .driver = {
        .name = ADXL345_NAME,
        .owner = THIS_MODULE,
    },
    .probe = adxl345_probe,
    .remove = adxl345_remove,
    .suspend = adxl345_suspend,
    .resume = adxl345_resume,
    .id_table = adxl345_id,
};

static int __init adxl345_init(void)
{
    return i2c_add_driver(&adxl345_driver);
}

static void __exit adxl345_exit(void)
{
    i2c_del_driver(&adxl345_driver);
}

module_init(adxl345_init);
module_exit(adxl345_exit);

MODULE_AUTHOR("Yamaha Corporation");
MODULE_DESCRIPTION("ADXL345 accelerometer driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(ADXL345_VERSION);

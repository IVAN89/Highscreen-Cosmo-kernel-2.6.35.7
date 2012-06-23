//Pochun_TKxxx_20101123_Begin
#include <linux/version.h>
#include <linux/kernel.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/ctype.h>
#include <linux/console.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/ioctl.h>

#include <mach/msm_iomap.h>

#include <linux/klog_collector.h>

MODULE_AUTHOR("Kevin Chiang <Kevin_Chiang@Compalcomm.com>");
MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("Kernel Log Collector");

/*******************************************************************************
* Local Variable/Structure Declaration
*******************************************************************************/
#define	KLOG_SIZE	(512 * 1024)
#define NAME_SIZE	32

/* Check Invalid Category */
#define CHK_CATEGORY(x) do { \
				if (x == KLOG_MAX_NUM) \
					return; \
				if (unlikely(x > KLOG_MAX_NUM)) { \
					printk(KERN_ERR "%s:%s():%u --> Func Caller:<%p> sent an Invalid KLog Category:%u\n", \
							__FILE__, __func__, __LINE__, \
							__builtin_return_address(0), \
							x); \
					return; /* FIXME: Kevin_Chiang@CCI: Too coupling coding style */ \
				} \
			} while (0)

struct cklog_t {
	unsigned char 		buffer[KLOG_SIZE];	/* ring buffer */
	unsigned char		name[NAME_SIZE];	/* log name */
	size_t				w_idx;				/* write index/offset of current log */
	unsigned int		overload;			/* 0:indicate the log read head is 0, 1:indicate the log read head in not 0(should be w_off+1) */
};

#if 1
#define MSM_KLOG_MAGIC  MSM_KLOG_BASE
#define MSM_KLOG_MAIN  MSM_KLOG_BASE + 100;
struct cklog_t * pcklog = (void *)  MSM_KLOG_MAIN;  /* The collection of all logs */
char *klog_magic =  (void *)  MSM_KLOG_MAGIC;	    /* magic number */
static int mem_ready = 0;			    /* 0: memory is not ready, 1: memory is ready */
static int mem_have_clean = 0;			    /* 0: memory is not clean, 1: memory has be clean */
static int magic_status = -1;			    /* -1: indicate magic number do not check yet, 0: indicate magic number is NULL, 1: indicate magic number is be set  */
#else
static struct cklog_t cklog[KLOG_MAX_NUM];
static char magic_str[100];
struct cklog_t * pcklog = (void *)  cklog;	    /* The collection of all logs */
char *klog_magic =  (void *) magic_str;		    /* magic number */
static int mem_ready = 1;                           /* 0: memory is not ready, 1: memory is ready */
static int mem_have_clean = 1;                      /* 0: memory is not clean, 1: memory has be clean */
static int magic_status = 0;                        /* -1: indicate magic number do not check yet, 0: indicate magic number is NULL, 1: indicate magic number is be set  */
#endif


/*******************************************************************************
* Local Function Declaration
*******************************************************************************/
int check_old_log(void);

/*******************************************************************************
* External Variable/Structure Declaration
*******************************************************************************/



/*******************************************************************************
* External Function Declaration
*******************************************************************************/



/*** Functions ***/
static __inline__ void __cklc_append_char(unsigned int category, unsigned char c)
{
	if(!mem_ready)
		return;

	if(check_old_log())
		return;

	if(!mem_have_clean){
		memset(pcklog, 0, sizeof(struct cklog_t) * KLOG_MAX_NUM);
		mem_have_clean = 1;
	}

	(pcklog+category)->buffer[(pcklog+category)->w_idx] = c;

	(pcklog+category)->w_idx = ((pcklog+category)->w_idx + 1) % KLOG_SIZE;

	if ((pcklog+category)->w_idx == 0)
		(pcklog+category)->overload = 1;

	return;
}

/* This func. should only be used in printk.c */
void cklc_append_kernel_raw_char(unsigned char c)
{

	__cklc_append_char(KLOG_KERNEL, c);

	return;
}
EXPORT_SYMBOL(cklc_append_kernel_raw_char);

/* Append One Character into Log[category] */
void cklc_append_char(unsigned int category, unsigned char c)
{
	CHK_CATEGORY(category);

	if (isprint(c) || c == '\0')
		__cklc_append_char(category, c);

	return;
}
EXPORT_SYMBOL(cklc_append_char);

static __inline__ void __cklc_append_str(unsigned int category, unsigned char *str, size_t len)
{
	int i = 0;

	for(i=0; i<len; i++) {
		if( isprint(*(str + i)) )
			__cklc_append_char(category, *(str + i));
	}

	return;
}

/* Append String into Log[category] */
void cklc_append_str(unsigned int category, unsigned char *str, size_t len)
{

	CHK_CATEGORY(category);
#if 0
	if (category == KLOG_ANDROID_EVENTS)
		printk(KERN_INFO "%s:%d --> Receive str:%s\n", __func__, __LINE__, str);
#endif
	__cklc_append_str(category, str, len);

	return;
}
EXPORT_SYMBOL(cklc_append_str);

static __inline__ void __cklc_append_newline(unsigned int category)
{
	__cklc_append_char(category, '\n');

	return;
}

/* Append New Line '\n' into Log[category] */
void cklc_append_newline(unsigned int category)
{
	CHK_CATEGORY(category);

	__cklc_append_newline(category);

	return;
}
EXPORT_SYMBOL(cklc_append_newline);

static __inline__ void __cklc_append_separator(unsigned int category)
{
	unsigned char *sp = " | ";

	__cklc_append_str(category, sp, strlen(sp));

	return;
}

/* Append Separator '|' into Log[category] */
void cklc_append_separator(unsigned int category)
{
	CHK_CATEGORY(category);

	__cklc_append_separator(category);

	return;
}
EXPORT_SYMBOL(cklc_append_separator);


static __inline__ void __cklc_append_time_header(unsigned int category)
{
	unsigned char tbuf[32];
	unsigned tlen = 0;
	struct timespec now;

	now = current_kernel_time();
	
	tlen = snprintf(tbuf, sizeof(tbuf), "[%8lx.%08lx] ", now.tv_sec, now.tv_nsec);

	__cklc_append_str(category, tbuf, tlen);

	return;
}

/* 
 * Append Unix Epoch Time as the Line Header 
 * to align with other Kernel, Andorid Log for easy time tracing
 */
void cklc_append_time_header(unsigned int category)
{
	CHK_CATEGORY(category);

	__cklc_append_time_header(category);

	return;
}
EXPORT_SYMBOL(cklc_append_time_header);

/* 
 * For Android Logger Version
 * drivers/misc/logger.c : d9312a065c888587f0cdd0e882bbac74bf734ea5
 */
void cklc_append_android_log(unsigned int category,
					const unsigned char *priority,
					const char * const tag,
					const int tag_bytes,
					const char * const msg,
					const int msg_bytes)
{
	int prilen = 0;
	unsigned char pribuf[8];

	__cklc_append_time_header(category);

	prilen = snprintf(pribuf, sizeof(pribuf), "<%u> ", (unsigned int)*priority);
	__cklc_append_str(category, pribuf, prilen);

	__cklc_append_str(category, (unsigned char *)tag, (unsigned int)tag_bytes);
	__cklc_append_separator(category);

	__cklc_append_str(category, (unsigned char *)msg, (unsigned int)msg_bytes);
	__cklc_append_newline(category);

	return;
}
EXPORT_SYMBOL(cklc_append_android_log);

static void console_output(unsigned char *buf, unsigned int len)
{
	struct console *con;

	for (con = console_drivers; con; con = con->next) {
		if ((con->flags & CON_ENABLED) && con->write &&
				(cpu_online(smp_processor_id()) ||
				(con->flags & CON_ANYTIME)))
			con->write(con, buf, len);
	}

	return;
}

static void __show_android_log_to_console(unsigned int category)
{
	unsigned int  len = 0;
	unsigned char strbuf[80];

	len = snprintf(strbuf, sizeof(strbuf), "\n\n============================= KLog Start =============================\n");
	console_output(strbuf, len);
	len = snprintf(strbuf, sizeof(strbuf), "KLog Category Name:[%u]%s\nKLog Ring Buffer:\n", category, (pcklog+category)->name);
	console_output(strbuf, len);

	if ((pcklog+category)->overload == 0 && (pcklog+category)->w_idx == 0) {
		len = snprintf(strbuf, sizeof(strbuf), "<Empty>");
		console_output(strbuf, len);
	} else {
		if ((pcklog+category)->overload == 0) {
			console_output(&(pcklog+category)->buffer[0], (pcklog+category)->w_idx);
		} else {
			console_output(&(pcklog+category)->buffer[(pcklog+category)->w_idx], KLOG_SIZE - (pcklog+category)->w_idx);
			console_output(&(pcklog+category)->buffer[0], (pcklog+category)->w_idx-1);
		}
	}

	len = snprintf(strbuf, sizeof(strbuf), "\n============================== KLog End ==============================\n");
	console_output(strbuf, len);

	return;
}

/* Show All Logs to All the Console Drivers. ex. UART */
void show_android_log_to_console(void)
{
	int i = 0;

	for(i=0; i<KLOG_MAX_NUM; i++) {
		__show_android_log_to_console(i);
	}

	return;
}
EXPORT_SYMBOL(show_android_log_to_console);

int check_old_log(void)
{
	if(magic_status == -1) {
		if (!strncmp(klog_magic, "PANIC", 5)){
			magic_status = 1;
		}
		else {
			magic_status = 0;
		}
	}
	return magic_status;
}

void cklc_save_log(void)
{
	sprintf(klog_magic,"PANIC");
}
EXPORT_SYMBOL(cklc_save_log);

void cklc_set_memory_ready(void)
{
    mem_ready = 1;
}
EXPORT_SYMBOL(cklc_set_memory_ready);

int klog_ioctl(struct inode *inode, struct file *filp,
		                unsigned int cmd, unsigned long arg)                            
{
	int flag;
	switch (cmd) {
		case REQUEST_KLOG_KERNEL:
			if (copy_to_user((void *)arg, pcklog + KLOG_KERNEL, sizeof(struct cklog_t)))
				return -EFAULT; 
			break;
		case REQUEST_KLOG_ANDROID_MAIN:
			if (copy_to_user((void *)arg, pcklog + KLOG_ANDROID_MAIN, sizeof(struct cklog_t)))
				return -EFAULT; 
			break;
		case REQUEST_KLOG_ANDROID_RADIO:
			if (copy_to_user((void *)arg, pcklog + KLOG_ANDROID_RADIO , sizeof(struct cklog_t)))
				return -EFAULT; 
			break;
		case REQUEST_PANIC:
			panic("klog panic\n");
			break;
		case CHECK_OLD_LOG:
			flag = check_old_log();
			if (copy_to_user((void *)arg, &flag ,sizeof(int)))
				return -EFAULT; 
			break;
		default:
			return -1;
	}

	return 0;
}

static const struct file_operations klog_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= klog_ioctl,
};

static struct miscdevice klog_miscdev = {
	.minor	= KLOG_MINOR,
	.name	= "cklc",
	.fops	= &klog_fops,
};

int klogmisc_init(void)
{
	int retval = 0;
	retval = misc_register(&klog_miscdev);
	if (retval) {
		printk(KERN_ERR "klog: cannot register miscdev on minor=%d (err=%d)\n", KLOG_MINOR, retval);
		goto outmisc;
	}

	return retval;

outmisc:
	misc_deregister(&klog_miscdev);
	return retval;
}

void klogmisc_exit(void)
{
	misc_deregister(&klog_miscdev);
}

static int __init cklc_init(void)
{
	int retval = 0;

	printk(KERN_INFO "CCI KLog Collector Init\n");

	//memset(pcklog, 0, sizeof(struct cklog_t) * KLOG_MAX_NUM);

	snprintf((pcklog+KLOG_KERNEL)->name, NAME_SIZE, "kernel");
	snprintf((pcklog+KLOG_ANDROID_MAIN)->name, NAME_SIZE, "android_main");
	snprintf((pcklog+KLOG_ANDROID_RADIO)->name, NAME_SIZE, "android_radio");
/*	snprintf(cklog[KLOG_ANDROID_EVENTS].name, NAME_SIZE, "android_events");*/

	retval = klogmisc_init();

	return retval;
}

static void __exit cklc_exit(void)
{
	printk(KERN_INFO "CCI KLog Collector Exit\n");
	klogmisc_exit();
	return;
}

module_init(cklc_init);
module_exit(cklc_exit);
//Pochun_TKxxx_20101123_End

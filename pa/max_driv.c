/* Necessary includes for a simple device driver */
#include<linux/init.h>
#include<linux/module.h>

#include<linux/slab.h> /* kmalloc() (initializing buffer) */

#include<linux/fs.h>
#include<asm/uaccess.h>
#define MAX_BUFFER_SIZE 1024

/* Declaration of max_driv.c functions */
int max_driv_open(struct inode *inode, struct file *filp);
int max_driv_release(struct inode *inode, struct file *filp);
ssize_t max_driv_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t max_driv_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void max_driv_exit(void);
int max_driv_init(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations max_driv_fops = {
	read: max_driv_read,
	write: max_driv_write,
	open: max_driv_open,
	release: max_driv_release
};

/* Declaration of the init and exit functions */
module_init(max_driv_init);
module_exit(max_driv_exit);


/* Global variables of the driver */
/* Major number */
int max_driv_major = 60;
/* Buffer to store data */
static char max_driv_buffer[MAX_BUFFER_SIZE];
/* buffer_size */
static unsigned long buffer_size = 0;
int open count = 0;
int closed_count = 0;


int max_driv_init(void) {
	int result;

	/* Registering device */
	result = register_chrdev(max_driv_major, "max_driv", &max_driv_fops);
	if (result < 0) {
    	printk("max_driv: cannot obtain major number %d\n", max_driv_major);
    	return result;
  	}
	
	printk("Inserting max_driv module\n");
	return 0;
}

void max_driv_exit(void) {
	/* Freeing the major number */
	unregister_chrdev(max_driv_major, "max_driv");

	printk("Removing max_driv module\n");
}

int max_driv_open(struct inode *inode, struct file *filp) {
	/* Success */
    open_count++;
    printk(KERN_ALERT "Device has been opened %d times", open_count);
	return 0;
}

int max_driv_release(struct inode *inode, struct file *filp) {
	/* When a file is closed, it's usually necessary to free
	the used memory and any variables related to the opening 
	of the device. But due to the simplicity of this example,
 	none of these operations are performed. */	
    
    closed_count++;
    printk(KERN_ALERT "Device has been closed %d times", closed_count);
    
	/* Success */
	return 0;
}

ssize_t max_driv_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	/* Count number of characters to read */
	int i = 0;
	while (max_driv_buffer[i] != '\0') i++;

	/* Transferring data to user space */
	copy_to_user(buf, max_driv_buffer, i);
	printk(KERN_ALERT "Read %d bytes including the null terminator", i);

	/* Changing reading position as best suits */
	if (*f_pos == 0) {
		*f_pos+=i;
		return i;
	}	else {
		return 0;
	}
}

ssize_t max_driv_write(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	
	/* get buffer size */
	buffer_size = count;
	if (buffer_size > MAX_BUFFER_SIZE) {
		buffer_size = MAX_BUFFER_SIZE;
	}

	/* Write data to buffer */
	copy_from_user(max_driv_buffer, buf, buffer_size);
	max_driv_buffer[buffer_size] = '\0'; /* Short strings need to be terminated */
	printk(KERN_ALERT "Wrote %d bytes including the null terminator", buffer_size);
	return buffer_size;
}








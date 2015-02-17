/*
 *  char driver  - Character Driver ($Revision: 1 $)
 *
 *  Copyright (C) 2015 Aswin Achuth Shankar  <achuthshankar@gmail.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <asm/uaccess.h>
dev_t dev;
int debug=1;   /*    To disable prints make debug as zero        */
#define BUFFER_SIZE 10
struct buffer
{
int no;
char device_buffer[BUFFER_SIZE];
struct buffer *link;
};

struct device
{
struct buffer *b;
struct cdev cdev;
};
struct device *dv;


/////////////////////////////////////////////////////////////////////////////////////////////////

int trim(struct device *d)
{
struct buffer *p,*b=d->b;
while(b)
{
p=b->link;
kfree(b);
b=p;
}
d->b=NULL;
return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int dev_open(struct inode *inode,struct file *filp)
{
	struct device *d;
	d=container_of(inode->i_cdev,struct device,cdev); 
	if(debug)
		printk("\nDevice opened\n");
	filp->private_data=d;
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY)
	{
		trim(d);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int dev_close(struct inode *inode,struct file *filp)
{
	if(debug)
		printk("\nDevice closed\n");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

ssize_t dev_read(struct file *filp,char __user *buff,ssize_t length,loff_t *pos)
{
	struct device *d=filp->private_data;
	struct buffer *b=d->b;
	if(!b)
	return 0;
	length=b->no;
	if(debug)
		printk("\nread starts");
	copy_to_user(buff,b->device_buffer, length);
	d->b=b->link;
	*pos+=length;
	if(debug)
		printk("\nread ends\n");
	kfree(b);
	return length;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

ssize_t dev_write(struct file *filp,const char __user *buff,size_t length,loff_t *pos)
{
	struct device *d=filp->private_data;
	struct buffer *temp,*b=d->b;
	if(debug)
		printk("\nwrite starts");
	if(length>BUFFER_SIZE)
	length=BUFFER_SIZE;
	temp=kmalloc(sizeof(struct buffer),GFP_KERNEL);
	temp->link=NULL;
	copy_from_user(temp->device_buffer, buff, length);
	temp->no=length;
	if(!*pos)
	d->b=temp;
	else
	{
	while(b->link)
	b=b->link;
	b->link=temp;
	}
	*pos+=length;
	if(debug)
		printk("\nwrite ends\n");
	return length;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

struct file_operations fops={
	.owner=THIS_MODULE, 
	.open=dev_open,        
	.read=dev_read,
	.write=dev_write,
	.release=dev_close,
};

/////////////////////////////////////////////////////////////////////////////////////////////////

int dev_init(void)
{
	int res;
	if(debug)
		printk(KERN_INFO"\ndev_init\n");
	res=alloc_chrdev_region(&dev,0,1,"my_char");
	if(res<0)
		printk(KERN_ALERT"\nDevice no Allocation failed\n");
	else
		printk(KERN_INFO"\nDevice no Allocation Successfull %d\n",MAJOR(dev));
	
	dv=kmalloc(sizeof(struct device),GFP_KERNEL);
	if(!dv){
	printk(KERN_INFO"\nMemory Allocation failed \n");
	return -ENOMEM;
	}
	memset(dv,0,sizeof(struct device));
	cdev_init(&dv->cdev,&fops);
	dv->cdev.owner=THIS_MODULE;
	dv->cdev.ops=&fops;	
	res=cdev_add(&dv->cdev,dev,1);
	if(res<0)
		printk(KERN_ALERT"\nAdding character device into the kernel failed\n");
	else
		printk(KERN_INFO"\nAdding character device into the kernel successfull\n");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void dev_exit(void)
{
	if(debug)
		printk(KERN_INFO"\ndev_exit\n");
	cdev_del(&dv->cdev);
	unregister_chrdev_region(dev,1);
	if(debug)
		printk(KERN_DEBUG"\nfreed Device no\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

module_init(dev_init);
module_exit(dev_exit);

/////////////////////////////////////////////////////////////////////////////////////////////////

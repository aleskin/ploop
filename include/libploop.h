/*
 *  Copyright (c) 2008-2017 Parallels International GmbH.
 *  Copyright (c) 2017-2019 Virtuozzo International GmbH. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _LIBPLOOP_H_
#define _LIBPLOOP_H_
#pragma GCC visibility push(default)

#include <sys/types.h>
#include <linux/types.h>

#define DISKDESCRIPTOR_XML	"DiskDescriptor.xml"
#define PLOOP_IMAGE_NAME	"root.hds"
#define QCOW_IMAGE_NAME		"root.hdd"

#define PLOOP_MAX_FS_SIZE	50ULL*1024*1024*1024*1024

#ifndef PLOOP_DEPRECATED
#define PLOOP_DEPRECATED __attribute__ ((deprecated))
#endif

enum ploop_image_mode {
	PLOOP_EXPANDED_MODE = 0,
	PLOOP_EXPANDED_PREALLOCATED_MODE = 1,
	PLOOP_RAW_MODE = 2,
};

/* flags for e2fsck() */
enum e2fsck_flags {
	E2FSCK_PREEN    = 1 << 0, /* -p */
	E2FSCK_FORCE    = 1 << 1, /* -f */
	E2FSCK_FORCE_REPAIR     = 1 << 2, /* -y */
	E2FSCK_RO	= 1 << 3, /* -n */
};


enum quota_type {
	PLOOP_JQUOTA	= 1,
	PLOOP_QUOTA	= 2,
};

struct ploop_mount_param {
	char device[64];	/* out: returns device name */
	int ro;			/* read-only mount */
	int flags;		/* flags such as MS_NOATIME */
	int noprobe;		/* do not partprobe device */
	char *fstype;		/* filesystem type; default if not set*/
	char *target;		/* mount point */
	char *guid;		/* UUID; top if not set */
	int quota;		/* enable inner FS quota with quota_type*/
	char *mount_data;
	unsigned int blocksize; /* blocksize for raw image */
	union {
		int fsck;
		int fsck_flags;	/* enum e2fsck_flags */
	};
	int fsck_rc;		/* out: fsck return code */
	int skip_balloon;
	int automount;
	unsigned long long balloon_ino;	/* out: balloon_ino */
	char dummy[32];
};

/* Bit values for ploop_create_param.flags */
enum ploop_create_flags {
	PLOOP_CREATE_NOLAZY		= 1 << 0, /* do NOT use lazy init */
	PLOOP_CREATE_SPARSE		= 2 << 0, /* create a sparse image  */
};

enum ploop_image_type {
	PLOOP_FMT	= 0,
	QCOW_FMT	= 1,
};

struct ploop_create_param {
	unsigned long long size;
	int mode;
	char *image;
	char *fstype;
	int without_partition;
	unsigned int blocksize;
	unsigned int fsblocksize;
	int fmt_version;
	unsigned int flags;
	const char *keyid;
	char *fslabel;
	int image_fmt;
	char dummy[32];
};

/* Bit values for ploop_replace_param.flag */
enum ploop_replace_flags {
	PLOOP_REPLACE_KEEP_NAME		= 1 << 0,
	PLOOP_REPLACE_RW		= 1 << 1,
};

struct ploop_replace_param {
	char *file;	/* A new image file name */
	/* Image to be replaced is specified by
	 * either guid, level, or current delta file */
	char *guid;
	int  level;
	char *cur_file;
	int  flags;
	int  mode;
	char dummy[28];
};

struct ploop_image_data {
	char *guid;
	char *file;
	int alien;
};

struct ploop_snapshot_data {
	char *guid;
	char *parent_guid;
	int temporary;
	int alien;
};

struct ploop_disk_images_runtime_data;

struct encryption_data {
	char *keyid;
};

struct volume_data {
	char *parent;
	char *snap_guid;
	int ro;
};

struct ploop_disk_images_data {
	unsigned long long size;
	unsigned int heads;
	unsigned int cylinders;
	unsigned int sectors;
	int mode;
	int nimages;
	struct ploop_image_data **images;
	char *top_guid;
	int nsnapshots;
	struct ploop_snapshot_data **snapshots;
	struct ploop_disk_images_runtime_data *runtime;
	unsigned int blocksize;
	unsigned long long max_delta_size;
	struct encryption_data *enc;
	struct volume_data *vol;
	char *cbt_uuid;
	char dummy[32];
};

struct ploop_resize_param {
	unsigned long long size;
	int offline_resize;
	pid_t mntns_pid;
	char dummy[32];
};

struct ploop_snapshot_param {
	char *guid;	/* guid for new snapshot, autogenerated if NULL */
	char *snap_dir; /* folder for new delta */
	char *cbt_uuid;
	char dummy[32];
};

struct ploop_tsnapshot_param {
	char *guid;
	char *component_name;
	char *target;
	char device[64];	/* out: assigned device */
	char *snap_dir;		/* folder for new delta */
	char *cbt_uuid;
	char dummy[32];
};


/* Values for ploop_snapshot_switch_param.flags field */
/* Do not remove old top delta */
#define PLOOP_SNAP_SKIP_TOPDELTA_DESTROY	0x01
/* Do not create a new delta after switching */
#define PLOOP_SNAP_SKIP_TOPDELTA_CREATE		0x02

struct ploop_snapshot_switch_param {
	const char *guid;
	/* guid for old top delta when SKIP_TOPDELTA_DESTROY is used */
	const char *guid_old;
	int flags;
	char dummy[32];
};

struct ploop_merge_param {
	int unused1;
	int merge_all;
	const char *guid;
	const char *unused2;
	const char *new_delta;
	char dummy[32];
};

struct ploop_discard_param {
	__u64 minlen_b;
	__u64 to_free;
	int automount;
	const volatile int *stop;
	int defrag;	/* 0-compact only, 1-defrag&compact, 2-defrag only */
	int image_defrag_threshold;
	char dummy[32];
};

struct ploop_info {
	unsigned long long fs_bsize;
	unsigned long long fs_blocks;
	unsigned long long fs_bfree;
	unsigned long long fs_inodes;
	unsigned long long fs_ifree;
};

struct ploop_fs_info
{
	struct ploop_info fs;
	char dev[64];
	char part[64];
};

struct ploop_mnt_info {
	int quota;	/* in */
	int ro;		/* in */
	char fstype[5];
	char uuid[38];
	const char *opts;
	char dummy[32];
};

struct ploop_spec {
	off_t size;
	__u32 blocksize;
	int fmt_version;
	char dummy[32];
};

struct ploop_discard_stat {
	off_t data_size;
	off_t ploop_size;
	off_t image_size;
	off_t balloon_size;
	int native_discard;
};

struct ploop_copy_receive_param {
	const char *file;	/* File name to write to */
	int ifd;		/* File descriptor to read from */
	int feedback_fd;	/* File descriptor to send feedback */
	char dummy[32];
};

struct ploop_copy_param {
	int ofd;
	int async;
	const char *device;
	int image_fmt;
	char dummy[28];
};

struct ploop_copy_stat {
	__u64 xferred_total;
	__u64 xferred;
};

enum {
	PLOOP_ENC_REENCRYPT	= 0x01,
	PLOOP_ENC_WIPE		= 0x02,
};

struct ploop_encrypt_param {
	const char *keyid;
	const char *mnt_opts;
	int flags;
	void *pad[4];
};

struct ploop_bitmap
{
	__u8 uuid[16];
	__u64 size_sec;
	__u64 cluster_sec;
	__u32 granularity_sec;
	__u32 l1_size;
	__u64 map[0];
};

struct ploop_tg_data {
	char devname[64];
	char devtg[64];
	char part[64];
	int lckfd;
};

struct ploop_tg_info {
	char status[64];
	int open_count;
	int ro;
	int blocksize;
};

struct ploop_compact_param {
	const char *path;
	int defrag;
	int dry;
	int threshold;
	int delta;
	volatile int *stop;
	volatile dev_t *compact_dev;
	char dummy[32];
};

/* Constants for ploop_set_verbose_level(): */
#define PLOOP_LOG_NOCONSOLE	-2	/* disable all console logging */
#define PLOOP_LOG_NOSTDOUT	-1	/* disable all but errors to stderr */
#define PLOOP_LOG_TIMESTAMPS	 4	/* enable sub-second timestamps */

#ifdef __cplusplus
extern "C" {
#endif

int ploop_set_component_name(struct ploop_disk_images_data *di,
		const char *component_name);
void ploop_set_umount_timeout(struct ploop_disk_images_data *di,	int timeout);
int ploop_get_top_delta_fname(struct ploop_disk_images_data *di, char *out, int len);
int ploop_get_base_delta_fname(struct ploop_disk_images_data *di, char *out, int len);
int ploop_find_dev(const char *component_name, const char *delta, char *buf, int size);
int ploop_get_dev_by_mnt(const char *path, char *buf, int size);
int ploop_get_mnt_by_dev(const char *dev, char *buf, int size);
int ploop_get_dev(struct ploop_disk_images_data *di, char *out, int len);
int ploop_get_part(struct ploop_disk_images_data *di, const char *dev,
                char *partname, int len);
int ploop_get_devname(struct ploop_disk_images_data *di, const char *dev,
		char *devname, int dlen, char *partname, int plen);
int ploop_get_devs(struct ploop_disk_images_data *di, char **out[]);
int ploop_is_mounted(struct ploop_disk_images_data *di);
void ploop_free_array(char *array[]);
int ploop_get_partition_by_mnt(const char *path, char *buf, int size);
int ploop_create_image(struct ploop_create_param *param);
int ploop_init_image(struct ploop_disk_images_data *di, struct ploop_create_param *param);
int ploop_init_device(const char *device, struct ploop_create_param *param);
int ploop_mount_image(struct ploop_disk_images_data *di, struct ploop_mount_param *param);
int ploop_mount_snapshot(struct ploop_disk_images_data *di, struct ploop_mount_param *param);
int ploop_umount(const char *device, struct ploop_disk_images_data *di);
int ploop_umount_image(struct ploop_disk_images_data *di);
int ploop_replace_image(struct ploop_disk_images_data *di, struct ploop_replace_param *param);
int ploop_resize_image(struct ploop_disk_images_data *di, struct ploop_resize_param *param);
int ploop_resize_blkdev(const char *device, off_t new_size);
int ploop_convert_image(struct ploop_disk_images_data *di, int mode, int flags);
int ploop_get_info_by_descr(const char *descr, struct ploop_info *info);
int ploop_create_snapshot(struct ploop_disk_images_data *di, struct ploop_snapshot_param *param);
int ploop_get_fs_info(const char *descr, struct ploop_fs_info *info, int size);
int ploop_create_temporary_snapshot(struct ploop_disk_images_data *di,
		struct ploop_tsnapshot_param *param, int *holder_fd);
int ploop_merge_snapshot(struct ploop_disk_images_data *di, struct ploop_merge_param *param);
int ploop_switch_snapshot_ex(struct ploop_disk_images_data *di, struct ploop_snapshot_switch_param *param);
int ploop_switch_snapshot(struct ploop_disk_images_data *di, const char *uuid, int flags);
int ploop_delete_snapshot(struct ploop_disk_images_data *di, const char *guid);

int ploop_delete_top_delta(struct ploop_disk_images_data *di);
int ploop_find_top_delta_name_and_format(
		const char *device,
		char *image,
		size_t image_size,
		char *format,
		size_t format_size);
char *ploop_find_parent_by_guid(struct ploop_disk_images_data *di, const char *guid);
int ploop_uuid_generate(char *uuid, int len);
int ploop_is_large_disk_supported(void);
int ploop_get_spec(struct ploop_disk_images_data *di, struct ploop_spec *spec);
int ploop_get_max_size(unsigned int blocksize, unsigned long long *max);
int ploop_set_max_delta_size(struct ploop_disk_images_data *di, unsigned long long size);

const char *ploop_get_last_error(void);
int ploop_set_log_file(const char *fname);
/* set log file logging level */
void ploop_set_log_level(int level);
/* set console logging level */
void ploop_set_verbose_level(int level);

/* Cancelation API */
void ploop_cancel_operation(void);
/* pcopy routines */
int ploop_copy_receiver(struct ploop_copy_receive_param *arg);

int ploop_discard_get_stat(struct ploop_disk_images_data *di,
		struct ploop_discard_stat *pd_stat);
int ploop_discard(struct ploop_disk_images_data *di,
			struct ploop_discard_param *param);

int ploop_compact(struct ploop_compact_param *param);
int ploop_open_dd(struct ploop_disk_images_data **di, const char *fname);
void ploop_close_dd(struct ploop_disk_images_data *di);
int ploop_create_dd(const char *ddxml, struct ploop_create_param *param);
int ploop_read_dd(struct ploop_disk_images_data *di);

int ploop_set_encryption_keyid(struct ploop_disk_images_data *di,
		const char *keyid);
int ploop_encrypt_image(struct ploop_disk_images_data *di,
		struct ploop_encrypt_param *param);
int ploop_drop_cbt(struct ploop_disk_images_data *di);
int ploop_clone_dd(struct ploop_disk_images_data *di, const char *guid,
	const char *target);
struct ploop_bitmap *ploop_get_used_bitmap_from_image(struct ploop_disk_images_data *di, const char *guid);
struct ploop_bitmap *ploop_get_tracking_bitmap_from_image(struct ploop_disk_images_data *di, const char *guid);
void ploop_release_bitmap(struct ploop_bitmap *bmap);
int ploop_get_names(const char *devname, char **names[]);
int ploop_dm_message(const char *devname, const char *msg, char **out);
void ploop_free_dm_message(char *msg);
int ploop_suspend_device(const char *devname);
int ploop_resume_device(const char *devname);
int ploop_image_defrag(const char *image, int flags);
int ploop_get_mnt_info(const char *partname, struct ploop_mnt_info *info);

int ploop_tg_init(const char *dev, const char *tg, struct ploop_tg_data *out);
int ploop_tg_deinit(const char *devtg, struct ploop_tg_data *data);
int ploop_tg_info(const char *devname, struct ploop_tg_info *param);


/* deprecated */
PLOOP_DEPRECATED char *ploop_get_base_delta_uuid(struct ploop_disk_images_data *di);
PLOOP_DEPRECATED int ploop_complete_running_operation(const char *device);

#ifdef __cplusplus
}
#endif


enum
{
	SYSEXIT_CREAT = 1,
	SYSEXIT_DEVICE,
	SYSEXIT_DEVIOC,
	SYSEXIT_OPEN,
	SYSEXIT_MALLOC,
	SYSEXIT_READ,
	SYSEXIT_WRITE,
	SYSEXIT_RESERVED_8,
	SYSEXIT_SYSFS,
	SYSEXIT_RESERVED_10,
	SYSEXIT_PLOOPFMT,
	SYSEXIT_SYS,
	SYSEXIT_PROTOCOL,
	SYSEXIT_LOOP,
	SYSEXIT_FSTAT,
	SYSEXIT_FSYNC,
	SYSEXIT_EBUSY,
	SYSEXIT_FLOCK,
	SYSEXIT_FTRUNCATE,
	SYSEXIT_FALLOCATE,
	SYSEXIT_MOUNT,
	SYSEXIT_UMOUNT,
	SYSEXIT_LOCK,
	SYSEXIT_MKFS,
	SYSEXIT_RESERVED_25,
	SYSEXIT_RESIZE_FS,
	SYSEXIT_MKDIR,
	SYSEXIT_RENAME,
	SYSEXIT_ABORT,
	SYSEXIT_RELOC,
	SYSEXIT_RESERVED_31,
	SYSEXIT_RESERVED_32,
	SYSEXIT_CHANGE_GPT,
	SYSEXIT_RESERVED_34,
	SYSEXIT_UNLINK,
	SYSEXIT_MKNOD,
	SYSEXIT_PLOOPINUSE,
	SYSEXIT_PARAM,
	SYSEXIT_DISKDESCR,
	SYSEXIT_DEV_NOT_MOUNTED,
	SYSEXIT_FSCK,
	SYSEXIT_RESERVED_42,
	SYSEXIT_NOSNAP,
	SYSEXIT_NOCBT	= 44,
	SYSEXIT_CRYPT,
	SYSEXIT_UMOUNT_BUSY,
};

#pragma GCC visibility pop
#endif

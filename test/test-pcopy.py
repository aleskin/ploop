#!/usr/bin/python3
import libploop
import shutil
import io
import os
import socket
import time
import subprocess as sp
import unittest
import hashlib
import sys

sleep_sec = 3
image_fmt = "qcow2"
#image_fmt = "ploop"

def hashfile(afile, hasher, blocksize=65536):
	buf = afile.read(blocksize)
	while len(buf) > 0:
		hasher.update(buf)
		buf = afile.read(blocksize)
	print (hasher.hexdigest())
	return hasher.hexdigest()

def get_devname():
	return '/dev/mapper/ploop1234'

def create_data():
	print("Fill data...")
	ret = sp.call(['/bin/dd', 'if=/dev/urandom', "of="+get_storage()+get_data(), 'bs=1M', 'count=512'])
	if ret != 0:
		raise Exception("Cannot create data file")
	print("Fill done")

def start_image_filller():
	pid = os.fork()
	if pid == 0:
		os.execl('/bin/dd', 'dd', "if="+get_storage()+get_data(), "of="+get_mnt()+get_data(), 'bs=1M', 'oflag=direct')
		os._exit(1)
	else:
		print("Start filler pid=%d" % pid)
		time.sleep(sleep_sec)
		return pid

def start_pcopy_receiver(fname, fd):
	print("Start receiver")
	t = libploop.ploopcopy_thr_receiver(fname, fd)
	t.start()
	return t

def get_storage():
	return '/vz/test/'

def get_data():
	return 'data.dat'

def get_image():
	return os.path.join(get_storage(), "test.hds")

def get_out():
	return os.path.join(get_storage(), "out.hds");

def get_ddxml():
	if image_fmt == "ploop":
		return os.path.join(get_storage(), 'DiskDescriptor.xml')
	return get_image()

def get_mnt():
	return get_storage() + "mnt/"

def ploop_create(img):
	ret = sp.call(["ploop", "init", "-s10g", "-T", image_fmt, img])
	if ret != 0:
		raise Exception("failed to create image")

def ploop_mount(ddxml):
	ret = sp.call(["ploop", "mount", "-m", get_mnt(), ddxml])
	if ret != 0:
		raise Exception("failed to mount image")

def ploop_umount(ddxml):
	ret = sp.run(["ploop", "list"], text=True, stdout=sp.PIPE)
	if ddxml.find("DiskDescriptor.xml") == -1:
		if ret.returncode == 0 and ret.stdout.find(ddxml) == -1:
			return 0
	else:
		if ret.returncode == 0 and len(ret.stdout) == 0:
			return 0
	return sp.call(["ploop", "umount", ddxml])

def dump_cbt(img):
	fout = img + ".cbt"
	
	with open(fout, "w") as f:
		p = sp.Popen(["ploop-cbt", "show", img], stdout=f)
	p.wait()
	return fout;

def do_ploop_copy(ddxml, fd):

	print("do_ploop_copy")
	ploop_mount(ddxml)
	pc = libploop.ploopcopy(ddxml, fd);

	pid = start_image_filller()

	print("Start copy")
	pc.copy_start()
	n = 0

	while True:
		print("Iter:",  n)
		transferred = pc.copy_next_iteration()
		print("transferred:", transferred)
		try:
			p, s = os.waitpid(pid, os.WNOHANG)
			if p == pid:
				break
		except:
			break;
		time.sleep(sleep_sec)
		n = n + 1
	try:
		os.waitpid(pid, 0)
	except:
		print("waitpid");

	print("Stop copy")
	pc.copy_stop()
	print("Umount")
	ploop_umount(ddxml)

def check(t):
	print("Check MD5");
	s = open(get_storage()+get_data(), 'rb')
	src = hashfile(s, hashlib.md5())

	ploop_mount(t.ddxml)
	d = open(get_mnt()+get_data(), 'rb')
	dst = hashfile(d, hashlib.md5())
	s.close()
	d.close()
	ploop_umount(t.ddxml)
	t.assertEqual(src, dst)

class testPcopy(unittest.TestCase):
	def setUp(self):
		if os.path.exists(get_image()):
			ploop_umount(get_image())
			shutil.rmtree(get_storage())

		if not os.path.exists(get_storage()):
			os.mkdir(get_storage())

		if not os.path.exists(get_mnt()):
			os.mkdir(get_mnt())

		create_data()
		ploop_create(get_image())
		self.out = get_out()
		self.ddxml = get_ddxml()

	def tearDown(self):
		print("tearDown")
		if os.path.exists(get_image()):
			ploop_umount(self.ddxml)
			shutil.rmtree(get_storage())

	def test_remote(self):
		print("Start remote")

		parent, child = socket.socketpair(socket.AF_UNIX, socket.SOCK_STREAM)
		self.rcv_thr = start_pcopy_receiver(self.out, child.fileno())

		if do_ploop_copy(self.ddxml, parent.fileno()):
			return
		parent.close()
		child.close()
		check(self)

"""
	def test_cbt(self):
		print("Start local CBT dst=%s" % self.out)

		ret = sp.call(["ploop", "snapshot", "-u262178fe-49d7-4c8b-b47c-4c0799dbf02a", "-b262178fe-49d7-4c8b-b47c-4c0799dbf02a", self.ddxml])
		if ret != 0:
			raise Exception("Cannot create snapshot")

		sp.call(["ploop", "snapshot-delete", "-u262178fe-49d7-4c8b-b47c-4c0799dbf02a", self.ddxml])

		parent, child = socket.socketpair(socket.AF_UNIX, socket.SOCK_STREAM)

		self.rcv_thr = start_pcopy_receiver(self.out, child.fileno())
		child.close()
		if do_ploop_copy(self.ddxml, parent.fileno()):
			return
		parent.close();

		f1 = dump_cbt(get_image())
		f2 = dump_cbt(self.out)
		print("Check CBT");
		ret = sp.call(["diff", "-u", f1, f2])
		if ret != 0:
			raise Exception("Check CBT failed")
		print("Check CBT [Ok]");
		check(self)
"""

"""
	def test_local(self):
		print("Start local")

		f = open(self.out, 'wb')
		if do_ploop_copy(self.ddxml, f.fileno()):
			return
		check(self)
		f.close()

"""
if __name__ == '__main__':
	unittest.main()

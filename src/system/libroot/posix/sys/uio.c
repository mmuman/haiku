/*
** Copyright 2004, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the MIT License.
*/


#include <syscalls.h>

#include <sys/uio.h>
#include <errno.h>

#include <errno_private.h>
#include <syscall_utils.h>


ssize_t
readv(int fd, const struct iovec *vecs, int count)
{
	ssize_t bytes;
	if (count < 0) {
		__set_errno(B_BAD_VALUE);
		return -1;
	}
	bytes = _kern_readv(fd, -1, vecs, count);

	RETURN_AND_SET_ERRNO(bytes);
}


ssize_t
readv_pos(int fd, off_t pos, const struct iovec *vecs, int count)
{
	ssize_t bytes;
	if (pos < 0 || count < 0) {
		__set_errno(B_BAD_VALUE);
		return -1;
	}
	bytes = _kern_readv(fd, pos, vecs, count);

	RETURN_AND_SET_ERRNO(bytes);
}


ssize_t
writev(int fd, const struct iovec *vecs, int count)
{
	ssize_t bytes;
	if (count < 0) {
		__set_errno(B_BAD_VALUE);
		return -1;
	}
	bytes = _kern_writev(fd, -1, vecs, count);

	RETURN_AND_SET_ERRNO(bytes);
}


ssize_t
writev_pos(int fd, off_t pos, const struct iovec *vecs, int count)
{
	ssize_t bytes;
	if (pos < 0 || count < 0) {
		__set_errno(B_BAD_VALUE);
		return -1;
	}
	bytes = _kern_writev(fd, pos, vecs, count);

	RETURN_AND_SET_ERRNO(bytes);
}


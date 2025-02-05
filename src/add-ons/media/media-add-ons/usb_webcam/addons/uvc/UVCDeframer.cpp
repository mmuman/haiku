/*
 * Copyright 2011, Gabriel Hartmann, gabriel.hartmann@gmail.com.
 * Distributed under the terms of the MIT License.
 */


#include "UVCDeframer.h"

#include "CamDebug.h"
#include "CamDevice.h"

#include <Autolock.h>


#define MAX_TAG_LEN CAMDEFRAMER_MAX_TAG_LEN
#define MAXFRAMEBUF CAMDEFRAMER_MAX_QUEUED_FRAMES


UVCDeframer::UVCDeframer(CamDevice* device)
	: CamDeframer(device),
	fFrameCount(0),
	fID(0)
{
}


UVCDeframer::~UVCDeframer()
{
}


ssize_t
UVCDeframer::Write(const void* buffer, size_t size)
{
	const uint8* buf = (const uint8*)buffer;
	int payloadSize = size;// - buf[0]; // total length - header length
	int headerSize = 0;

	// check if we really have a header
	printf("buf[0] %02x\n", buf[0]);
	if ((size > 2) && (buf[0] > 1) && (buf[0] <= 12) && (buf[0] <= size)) {
		int expectedLen = 2;
		if (buf[1] & (1 << 2))
			expectedLen += 4; // PTS
		if (buf[1] & (1 << 3))
			expectedLen += 6; // SCR
		// header length match the flags, reserved bit is 0
		if ((buf[0] == expectedLen) && !(buf[1] & 1 << 4)) {
			// Seems like a valid header
			headerSize = buf[0];
			payloadSize -= buf[0];
			printf("Got %d bytes header.\n", headerSize);
			if (buf[1] & (1 << 2)) {
				;// TODO: use Presentation Time Stamp
			}
			if (buf[1] & (1 << 3)) {
				;// TODO: use Source Clock Reference
			}
		}
	}
	
	// This packet is just a header
	if (headerSize && (size == buf[0]))
		return 0;

	printf("Got %d bytes payload.\n", payloadSize);
	// Should not happen
	if (payloadSize < 0)
		return 0;

	// Allocate frame
	if (!fCurrentFrame) {
		BAutolock l(fLocker);
		if (fFrames.CountItems() < MAXFRAMEBUF)
			fCurrentFrame = AllocFrame();
		else {
			printf("Dropped %ld bytes. Too many queued frames.)\n", size);
			return size;
		}
	}
	
	// Write payload to buffer
	fInputBuffer.Write(&buf[headerSize], payloadSize);
	
	// If end of frame add frame to list of frames
	if (headerSize && ((buf[1] & 2) || (buf[1] & 1) != fID)) {
		fID = buf[1] & 1;
		fFrameCount++;
		buf = (uint8*)fInputBuffer.Buffer();
		fCurrentFrame->Write(buf, fInputBuffer.BufferLength());
		BAutolock l(fLocker);
		fFrames.AddItem(fCurrentFrame);
		release_sem(fFrameSem);
		fCurrentFrame = NULL;
	}

	return size;
}


void
UVCDeframer::_PrintBuffer(const void* buffer, size_t size)
{
	uint8* b = (uint8*)buffer;
	for (size_t i = 0; i < size; i++)
		printf("0x%x\t", b[i]);
	printf("\n");
}


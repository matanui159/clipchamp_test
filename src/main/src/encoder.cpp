#include "encoder.h"
#include <svc/codec_api.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct encoder_t {
	ISVCEncoder* svc;
	SSourcePicture pic;
	SFrameBSInfo frm;

	int time;
	byte_t* data;
	int size;
};

encoder_t* encoder_create(int width, int height, int fps) {
	encoder_t* enc = new encoder_t;
	int result;

	result = WelsCreateSVCEncoder(&enc->svc);
	assert(result == 0);

	SEncParamBase params;
	params.iPicWidth = width;
	params.iPicHeight = height;
	params.fMaxFrameRate = fps;
	params.iUsageType = CAMERA_VIDEO_REAL_TIME;
	params.iTargetBitrate = 10000000;
	params.iRCMode = RC_QUALITY_MODE;
	result = enc->svc->Initialize(&params);
	assert(result == 0);

	enc->pic.iColorFormat = videoFormatI420;
	enc->pic.iPicWidth = width;
	enc->pic.iPicHeight = height;
	enc->pic.uiTimeStamp = 0;
	enc->pic.iStride[0] = width;
	enc->pic.iStride[1] = width / 2;
	enc->pic.iStride[2] = width / 2;

	enc->time = 1000 / fps;
	enc->data = NULL;
	enc->size = 0;

	memset(&enc->frm, 0, sizeof(SFrameBSInfo));
	return enc;
}

void encoder_destroy(encoder_t* enc) {
	free(enc->data);
	enc->svc->Uninitialize();
	WelsDestroySVCEncoder(enc->svc);
	delete enc;
}

static int layer_size(SLayerBSInfo* layer) {
	int size = 0;
	for (int i = 0; i < layer->iNalCount; ++i) {
		size += layer->pNalLengthInByte[i];
	}
	return size;
}

void encoder_frame(encoder_t* enc, byte_t* data) {
	int width = enc->pic.iPicWidth;
	int height = enc->pic.iPicHeight;
	enc->pic.pData[0] = data;
	enc->pic.pData[1] = enc->pic.pData[0] + width * height;
	enc->pic.pData[2] = enc->pic.pData[1] + (width * height / 4);

	int result = enc->svc->EncodeFrame(&enc->pic, &enc->frm);
	assert(result == 0);
	if (enc->frm.eFrameType != videoFrameTypeSkip) {
		int size = 0;
		for (int i = 0; i < enc->frm.iLayerNum; ++i) {
			size += layer_size(enc->frm.sLayerInfo + i);
		}

		enc->data = static_cast<byte_t*>(realloc(enc->data, enc->size + size));
		for (int i = 0; i < enc->frm.iLayerNum; ++i) {
			SLayerBSInfo* layer = enc->frm.sLayerInfo + i;
			size = layer_size(layer);
			memcpy(enc->data + enc->size, layer->pBsBuf, size);
			enc->size += size;
		}
	}

	enc->pic.uiTimeStamp += enc->time;
}

const byte_t* encoder_data(encoder_t* enc, int* size) {
	*size = enc->size;
	return enc->data;
}